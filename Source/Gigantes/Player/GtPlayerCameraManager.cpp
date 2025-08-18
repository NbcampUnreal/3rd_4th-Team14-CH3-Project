// Fill out your copyright notice in the Description page of Project Settings.


#include "GtPlayerCameraManager.h"

#include "GtCameraModifierSource.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Gigantes/GtGameplayTags.h"
#include "Gigantes/Character/GtHeroCharacter.h"
#include "Gigantes/Character/Components/GtHeroMovementComponent.h"
#include "Gigantes/Equipments/Components/GtLoadoutComponent.h"
#include "Gigantes/Items/Weapons/GtWeaponItem.h"

AGtPlayerCameraManager::AGtPlayerCameraManager()
{
    BaseStatePriority =
    {
        GtGameplayTags::Status_Action_Crouching,
        GtGameplayTags::Status_Action_WallRunning,
        GtGameplayTags::Status_Action_Sliding
    };
}

void AGtPlayerCameraManager::SetViewTarget(AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams)
{
    if (CachedHeroCharacter.IsValid())
    {
        CachedHeroCharacter->OnStatusTagChanged.RemoveDynamic(this, &AGtPlayerCameraManager::OnCharacterStatusTagChanged);
    }

    Super::SetViewTarget(NewViewTarget, TransitionParams);
    
    // 새로운 ViewTarget이 HeroCharacter인 경우 초기화
    if (AGtHeroCharacter* HeroCharacter = Cast<AGtHeroCharacter>(NewViewTarget))
    {
        CachedHeroCharacter = HeroCharacter;
        
        if (UGtHeroMovementComponent* HeroMovementComp = HeroCharacter->GetHeroMovementComponent())
        {
            // 캐릭터의 실제 캡슐 높이 값 캐싱
            CachedStandingCapsuleHalfHeight = HeroCharacter->GetDefaultHalfHeight();

            HeroCharacter->OnStatusTagChanged.AddDynamic(this, &AGtPlayerCameraManager::OnCharacterStatusTagChanged);

            PreviousStatusTags = HeroCharacter->GetStatusTags();
            UpdateCameraTargets();

            CurrentCameraOffsetZ = TargetCameraOffsetZ;
            CurrentFOV = TargetFOV;
            CurrentSpringArmLength = TargetSpringArmLength;
        }
    }
}

void AGtPlayerCameraManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (CachedHeroCharacter.IsValid())
    {
        CachedHeroCharacter->OnStatusTagChanged.RemoveDynamic(this, &AGtPlayerCameraManager::OnCharacterStatusTagChanged);
    }
    
    Super::EndPlay(EndPlayReason);
}

void AGtPlayerCameraManager::OnCharacterStatusTagChanged(const FGameplayTag& StatusTag, bool bAdded)
{
    // 태그가 추가되었을 때
    if (bAdded)
    {
        // 현재는 무기에 의한 모디파이어만 처리
        AGtHeroCharacter* Hero = GetHeroCharacter();
        if (Hero && Hero->GetLoadoutComponent())
        {
            AGtWeaponItem* CurrentWeapon = Hero->GetLoadoutComponent()->GetCurrentEquippedWeapon();
            if (CurrentWeapon && CurrentWeapon->Implements<UGtCameraModifierSource>())
            {
                FGtCameraModifier Modifier;
                // 현재 장착한 무기가 이 태그에 대한 모디파이어를 가지고 있는지 확인
                if (IGtCameraModifierSource::Execute_GetCameraModifierForTag(CurrentWeapon, StatusTag, Modifier))
                {
                    // 있다면 동적 모디파이어로 등록
                    SetDynamicModifier(StatusTag, Modifier);
                }
            }
        }
    }
    // 태그가 제거되었을 때
    else
    {
        // 해당 태그로 등록된 모디파이어를 제거
        RemoveDynamicModifier(StatusTag);
    }
    
    // 액션 태그라면 카메라 목표 값을 재계산
    if (StatusTag.MatchesTag(GtGameplayTags::Status_Action))
    {
        UpdateCameraTargets();
    }
}

FGameplayTag AGtPlayerCameraManager::DetermineBaseState(const AGtHeroCharacter* HeroCharacter) const
{
    if (!HeroCharacter)
    {
        return FGameplayTag::EmptyTag;
    }
    
    // 우선순위에 따라 베이스 상태 결정 (뒤에서부터 체크 - 높은 우선순위)
    for (int32 i = BaseStatePriority.Num() - 1; i >= 0; i--)
    {
        if (HeroCharacter->HasStatusTag(BaseStatePriority[i]))
        {
            return BaseStatePriority[i];
        }
    }
    
    return FGameplayTag::EmptyTag;
}

void AGtPlayerCameraManager::ApplyModifiers(const FGameplayTagContainer& StatusTags)
{
    // 모디파이어를 우선순위에 따라 정렬
    TArray<FModifierPair> ActiveModifiers;
    
    for (const auto& Pair : DynamicModifierMap)
    {
        if (StatusTags.HasTagExact(Pair.Key))
        {
            FModifierPair ModPair;
            ModPair.Tag = Pair.Key;
            ModPair.Modifier = Pair.Value;
            ActiveModifiers.Add(ModPair);
        }
    }
    
    // Priority에 따라 정렬 (낮은 것부터 높은 순으로)
    ActiveModifiers.Sort([](const FModifierPair& A, const FModifierPair& B)
    {
        return A.Modifier.Priority < B.Modifier.Priority;
    });
    
    // 정렬된 순서대로 모디파이어 적용
    for (const FModifierPair& ModPair : ActiveModifiers)
    {
        const FGtCameraModifier& Modifier = ModPair.Modifier;
        
        // Camera Offset Z
        if (Modifier.CameraOffsetZ_Op == EGtCameraValueOperation::Additive)
        {
            TargetCameraOffsetZ += Modifier.CameraOffsetZ;
        }
        else
        {
            TargetCameraOffsetZ = Modifier.CameraOffsetZ;
        }
        
        // FOV
        if (Modifier.FOV_Op == EGtCameraValueOperation::Additive)
        {
            TargetFOV += Modifier.FOV;
        }
        else
        {
            TargetFOV = Modifier.FOV;
        }
        
        // Spring Arm Length
        if (Modifier.SpringArmLength_Op == EGtCameraValueOperation::Additive)
        {
            TargetSpringArmLength += Modifier.SpringArmLength;
        }
        else
        {
            TargetSpringArmLength = Modifier.SpringArmLength;
        }
        
        // 전환 속도는 가장 마지막 모디파이어의 값 사용
        CurrentTransitionSpeed = Modifier.TransitionSpeed;
    }
}

void AGtPlayerCameraManager::UpdateCameraTargets()
{
    AGtHeroCharacter* HeroCharacter = GetHeroCharacter();
    if (!HeroCharacter)
    {
        return;
    }

    // 베이스 상태 결정
    FGameplayTag NewBaseStateTag = DetermineBaseState(HeroCharacter);
    
    // 상태 변경 감지
    const FGameplayTagContainer& CurrentStatusTags = HeroCharacter->GetStatusTags();
    bool bStateChanged = (NewBaseStateTag != CurrentBaseStateTag) || (CurrentStatusTags != PreviousStatusTags);
    
    if (!bStateChanged)
    {
        return; // 변경사항 없으면 조기 종료
    }
    
    CurrentBaseStateTag = NewBaseStateTag;
    PreviousStatusTags = CurrentStatusTags;

    RecalculateTargets();
}

void AGtPlayerCameraManager::RecalculateTargets()
{
    AGtHeroCharacter* HeroCharacter = GetHeroCharacter();
    if (!HeroCharacter)
    {
        return;
    }

    // 베이스 옵션 설정
    const FGtCameraOption* BaseOptions = BaseCameraOptionsMap.Find(CurrentBaseStateTag);
    if (BaseOptions)
    {
        TargetCameraOffsetZ = BaseOptions->CameraOffsetZ;
        TargetFOV = BaseOptions->FOV;
        TargetSpringArmLength = BaseOptions->SpringArmLength;
        CurrentTransitionSpeed = BaseOptions->TransitionSpeed;
    }
    else
    {
        // 기본 옵션 사용
        TargetCameraOffsetZ = DefaultCameraOptions.CameraOffsetZ;
        TargetFOV = DefaultCameraOptions.FOV;
        TargetSpringArmLength = DefaultCameraOptions.SpringArmLength;
        CurrentTransitionSpeed = DefaultCameraOptions.TransitionSpeed;
    }
    
    // 모디파이어 적용
    const FGameplayTagContainer& CurrentStatusTags = HeroCharacter->GetStatusTags();
    ApplyModifiers(CurrentStatusTags);
}

void AGtPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
    Super::UpdateViewTarget(OutVT, DeltaTime);

    // TODO: 추후 로못 캐릭터 구현 시 이렇게 Hero전용으로 하면 안됨
    if (GetHeroCharacter())
    {
        ApplyCameraBehavior(OutVT, DeltaTime);
        UpdateAimOffset(OutVT, DeltaTime);
    }
}

void AGtPlayerCameraManager::ApplyCameraBehavior(FTViewTarget& OutVT, float DeltaTime)
{
    AGtHeroCharacter* HeroCharacter = GetHeroCharacter();
    if (!HeroCharacter)
    {
        return;
    }
    
    // 캡슐 높이 변화에 대해 원래 높이로 맞춰줌
    const float CurrentCapsuleHalfHeight = HeroCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    const float CapsuleHeightDifference = CachedStandingCapsuleHalfHeight - CurrentCapsuleHalfHeight;
    OutVT.POV.Location += HeroCharacter->GetActorUpVector() * CapsuleHeightDifference;

    // 부드럽게 카메라 높이 보간
    CurrentCameraOffsetZ = FMath::FInterpTo(CurrentCameraOffsetZ, TargetCameraOffsetZ, DeltaTime, CurrentTransitionSpeed);
    CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, CurrentTransitionSpeed);
    CurrentSpringArmLength = FMath::FInterpTo(CurrentSpringArmLength, TargetSpringArmLength, DeltaTime, CurrentTransitionSpeed);

    // ViewTarget에 최종 view 정보 적용
    OutVT.POV.Location += HeroCharacter->GetActorUpVector() * CurrentCameraOffsetZ;
    OutVT.POV.FOV = CurrentFOV;

    if (USpringArmComponent* SpringArm = HeroCharacter->GetCameraBoom())
    {
        SpringArm->TargetArmLength = CurrentSpringArmLength;
    }
}

void AGtPlayerCameraManager::UpdateAimOffset(const FTViewTarget& VT, float DeltaTime)
{
    AGtHeroCharacter* HeroCharacter = GetHeroCharacter();
    if (!HeroCharacter || !HeroCharacter->Controller)
    {
        CurrentAimOffsetYaw = 0.0f;
        CurrentAimOffsetPitch = 0.0f;
        return;
    }

    const FRotator ControlRotation = VT.POV.Rotation;
    const FRotator CharacterRotation = HeroCharacter->GetActorRotation();

    const float TargetYaw = FRotator::NormalizeAxis(ControlRotation.Yaw - CharacterRotation.Yaw);
    const float TargetPitch = FRotator::NormalizeAxis(ControlRotation.Pitch);
    
    CurrentAimOffsetYaw = TargetYaw;
    CurrentAimOffsetPitch = TargetPitch;
}

AGtHeroCharacter* AGtPlayerCameraManager::GetHeroCharacter() const
{
    return Cast<AGtHeroCharacter>(GetViewTarget());
}

void AGtPlayerCameraManager::SetDynamicModifier(const FGameplayTag& Tag, const FGtCameraModifier& Modifier)
{
    DynamicModifierMap.Add(Tag, Modifier);
}

void AGtPlayerCameraManager::RemoveDynamicModifier(const FGameplayTag& Tag)
{
    DynamicModifierMap.Remove(Tag);
}

void AGtPlayerCameraManager::ClearAllDynamicModifiers()
{
    DynamicModifierMap.Empty();
    RecalculateTargets();
}
