// Fill out your copyright notice in the Description page of Project Settings.


#include "GtTestWeaponBase.h"

#include "TestGtGameplayTags.h"
#include "Components/SphereComponent.h"
#include "Gigantes/GtGameplayTags.h"
#include "Gigantes/Character/GtHeroCharacter.h"
#include "Gigantes/Gameplay/Damage/GtDamageable.h"
#include "Gigantes/Physics/GtCollisionChannels.h"


AGtTestWeaponBase::AGtTestWeaponBase()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentAmmo = TestMaxAmmo;

    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(Sphere);
}

void AGtTestWeaponBase::InitializeTestWeapon()
{
    // 테스트 데이터 직접 설정
    ItemData.ItemId = "test_rifle_01";
    ItemData.ItemName = "Test Rifle";
    ItemData.ItemTag = GtGameplayTags::Item_Weapon_TestRifle;
    ItemData.Damage = 30;
    ItemData.MaxAmmo = 30;
    ItemData.AmmoInMagazine = 30;
    ItemData.FireRate = 0.1f;
    ItemData.ReloadTime = 2.0f;
    
    // AGtWeaponItem의 멤버 변수들도 설정
    Damage = ItemData.Damage;
    MaxAmmo = ItemData.MaxAmmo;
    AmmoInMagazine = ItemData.AmmoInMagazine;
    FireRate = ItemData.FireRate;
    ReloadRate = ItemData.ReloadTime;

    // 테스트용 조준 설정
    bCanAim = true;
    AimMovementSpeedMultiplier = 0.45f;
    
    // 조준 카메라 모디파이어 설정
    AimCameraModifier.Priority = 100;
    AimCameraModifier.FOV_Op = EGtCameraValueOperation::Override;
    AimCameraModifier.FOV = 60.0f;
    AimCameraModifier.SpringArmLength_Op = EGtCameraValueOperation::Additive;
    AimCameraModifier.SpringArmLength = -40.0f;
    AimCameraModifier.CameraOffsetZ_Op = EGtCameraValueOperation::Additive;
    AimCameraModifier.CameraOffsetZ = 0.0f;
    AimCameraModifier.TransitionSpeed = 15.0f;
}

void AGtTestWeaponBase::OnEquipped_Implementation(AActor* NewOwner)
{
    WeaponOwner = NewOwner;
    CurrentAmmo = TestMaxAmmo;
    bCanFire = true;
    
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(NewOwner))
    {
        if (ArmedAnimLayer)
        {
            OwnerCharacter->GetMesh()->GetAnimInstance()->LinkAnimClassLayers(ArmedAnimLayer);
            UE_LOG(LogTemp, Log, TEXT("Armed Anim Layer Linked."));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[TestWeapon] Equipped to %s"), 
        NewOwner ? *NewOwner->GetName() : TEXT("Unknown"));
}

void AGtTestWeaponBase::OnUnequipped_Implementation()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AutoFireTimerHandle);
    }
    
    bFireInputPressed = false;
    
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(WeaponOwner))
    {
        if (bIsAiming)
        {
            StopAiming();
        }
        if (ArmedAnimLayer)
        {
            OwnerCharacter->GetMesh()->GetAnimInstance()->UnlinkAnimClassLayers(ArmedAnimLayer);
            UE_LOG(LogTemp, Log, TEXT("Armed Anim Layer Unlinked."));
        }
    }

    WeaponOwner = nullptr;
    GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
    
    UE_LOG(LogTemp, Warning, TEXT("[TestWeapon] Unequipped"));
}

void AGtTestWeaponBase::ExecutePrimaryActionPressed_Implementation()
{
    bFireInputPressed = true;
    
    switch (FireMode)
    {
    case EGtFireMode::Single:
        // 단발 모드: bCanFire로 쿨다운을 직접 제어
        if (bCanFire)
        {
            bCanFire = false;
            TestFire(); 
            
            // 쿨다운 타이머 설정
            GetWorld()->GetTimerManager().SetTimer(
                FireTimerHandle,
                [this]() { bCanFire = true; },
                TestFireRate,
                false
            );
        }
        break;
        
    case EGtFireMode::Automatic:
        // 자동 모드: 반복 타이머가 연사 속도를 제어하므로 bCanFire가 필요 없음
        TestFire(); // 첫 발 즉시 발사
        if (GetWorld() && TestFireRate > 0.0f)
        {
            GetWorld()->GetTimerManager().SetTimer(
                AutoFireTimerHandle,
                this,
                &AGtTestWeaponBase::TestFire,
                TestFireRate,
                true  
            );
        }
        break;
        
    case EGtFireMode::Burst:
        // 점사 모드 (선택사항)
        // TODO: 3발씩 발사하는 로직
        break;
    }
}

void AGtTestWeaponBase::ExecutePrimaryActionReleased_Implementation()
{
    bFireInputPressed = false;
    
    // 자동 발사 타이머 정지
    if (FireMode == EGtFireMode::Automatic)
    {
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(AutoFireTimerHandle);
        }
    }
}

void AGtTestWeaponBase::ExecuteSecondaryActionPressed_Implementation()
{
    if (bUseHoldToAim)
    {
        // 홀딩 모드: 누를 때 시작
        if (!bIsAiming)
        {
            StartAiming();
        }
    }
    else
    {
        // 토글 모드: 누를 때 토글
        if (bIsAiming)
        {
            StopAiming();
        }
        else
        {
            StartAiming();
        }
    }
}

void AGtTestWeaponBase::ExecuteSecondaryActionReleased_Implementation()
{
    if (bUseHoldToAim && bIsAiming)
    {
        // 홀딩 모드에서만 뗄 때 해제
        StopAiming();
    }
    // 토글 모드에서는 Release 시 아무것도 안 함
}

void AGtTestWeaponBase::StartAiming()
{
    // TODO: 추후 ItemData.bCanAim으로 체크 변경
    if (!bCanAim || !WeaponOwner)
        return;

    AGtHeroCharacter* Hero = Cast<AGtHeroCharacter>(WeaponOwner);
    if (!Hero)
    {
        return;
    }
    
    bIsAiming = true;

    Hero->AddStatusTag(GtGameplayTags::Status_Action_Aiming);
    Hero->bUseAimOffset = true;
    
    UE_LOG(LogTemp, Log, TEXT("[TestWeapon] Start Aiming"));
}

void AGtTestWeaponBase::StopAiming()
{
    if (!bIsAiming || !WeaponOwner)
        return;

    bIsAiming = false;
    
    if (AGtHeroCharacter* Hero = Cast<AGtHeroCharacter>(WeaponOwner))
    {
        Hero->RemoveStatusTag(GtGameplayTags::Status_Action_Aiming);
        Hero->bUseAimOffset = false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("[TestWeapon] Stop Aiming"));
}

void AGtTestWeaponBase::ExecuteReloadAction_Implementation()
{
    TestReload();
}

bool AGtTestWeaponBase::GetCameraModifierForTag_Implementation(const FGameplayTag& ActionTag,
    FGtCameraModifier& OutModifier) const
{
    // 요청받은 태그가 조준 태그이고 이 무기가 조준을 할 수 있다면
    if (ActionTag == GtGameplayTags::Status_Action_Aiming && bCanAim)
    {
        // 무기가 가진 조준용 모디파이어를 넘겨주고 true 반환
        OutModifier = AimCameraModifier;
        return true;
    }
    // 그 외의 경우에는 이 무기는 관련 모디파이어가 없으므로 false 반환
    return false;
}

void AGtTestWeaponBase::TestFire()
{
    // 자동 발사 중 탄약이 떨어지면 타이머 정지
    if (CurrentAmmo <= 0 || !WeaponOwner)
    {
        if (FireMode == EGtFireMode::Automatic)
        {
            GetWorld()->GetTimerManager().ClearTimer(AutoFireTimerHandle);
        }
        UE_LOG(LogTemp, Warning, TEXT("[TestWeapon] Cannot fire: No Ammo or Owner"));
        return;
    }

    // TODO : 멤버 변수로 캐싱 대신에 GetOwner 사용 고려
    APawn* OwnerPawn = Cast<APawn>(WeaponOwner);
    if (!OwnerPawn)
        return;
    
    CurrentAmmo--;

    // 크로스헤어 LineTrace 결과를 그대로 사용
    FHitResult HitResult;
    bool bHit = GetCrosshairHitResult(HitResult);
    
    // 디버그 표시
    if (bShowDebugLine)
    {
        // 시작점에서 히트 지점까지 라인
        FVector TraceStart = HitResult.TraceStart;
        FVector TraceEnd = bHit ? HitResult.Location : HitResult.TraceEnd;
        
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, 
            bHit ? FColor::Green : FColor::Red, 
            false, DebugLineDuration, 0, 2.0f);
        
        if (bHit)
        {
            // 히트 포인트
            DrawDebugSphere(GetWorld(), HitResult.Location, 
                10.0f, 12, FColor::Yellow, false, DebugLineDuration);
        }
    }
    
    // 데미지 처리
    if (bHit && HitResult.GetActor())
    {
        if (HitResult.GetActor()->Implements<UGtDamageable>())
        {
            FGtDamageInfo DamageInfo;
            DamageInfo.BaseDamage = TestDamage;
            DamageInfo.DamageCauser = this;
            DamageInfo.Instigator = OwnerPawn->GetController();
            DamageInfo.HitResultInfo = HitResult;
            
            FGtDamageResult DamageResult;
            IGtDamageable::Execute_ApplyDamage(HitResult.GetActor(), DamageInfo, DamageResult);
            
            UE_LOG(LogTemp, Warning, TEXT("[TestWeapon] Hit %s for %.1f damage"), 
                *HitResult.GetActor()->GetName(), DamageResult.FinalDamage);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[TestWeapon] Fire! Ammo: %d/%d"), CurrentAmmo, TestMaxAmmo);
    
}

void AGtTestWeaponBase::TestReload()
{
    CurrentAmmo = TestMaxAmmo;
    UE_LOG(LogTemp, Warning, TEXT("[TestWeapon] Reloaded! Ammo: %d/%d"), CurrentAmmo, TestMaxAmmo);
}

bool AGtTestWeaponBase::GetCrosshairHitResult(FHitResult& OutHitResult) const
{
    APawn* OwnerPawn = Cast<APawn>(WeaponOwner);
    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
    
    // 카메라 정보
    FVector CamLoc;
    FRotator CamRot;
    PC->GetPlayerViewPoint(CamLoc, CamRot);
    FVector AimDir = CamRot.Vector().GetSafeNormal();
    
    // 초점 거리
    const double FocalDistance = 1024.0f;
    FVector FocalLoc = CamLoc + (AimDir * FocalDistance);
    
    // 폰 위치를 조준선에 투영
    const FVector PawnLoc = OwnerPawn->GetActorLocation();
    FVector ProjectedStartLoc = FocalLoc + (((PawnLoc - FocalLoc) | AimDir) * AimDir);
    
    // 투영된 위치에서 조준 방향으로 LineTrace (데미지 판정용)
    FVector Start = ProjectedStartLoc;
    FVector End = Start + AimDir * 10000.0f;
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(WeaponOwner);
    QueryParams.AddIgnoredActor(this);
    //QueryParams.bTraceComplex = true;  // 정확한 히트 위치
    //QueryParams.bReturnPhysicalMaterial = true;  // 물리 머티리얼 (헤드샷 등)
    
    // ECC_Visibility로 모든 것 체크 (벽, 적 등)
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        OutHitResult,
        Start, 
        End, 
        Gt_TraceChannel_Weapon_Capsule, 
        QueryParams
    );
    
    return bHit;
}

