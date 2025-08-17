#include "GtTestWeaponBase.h"

#include "TestGtGameplayTags.h"
#include "Components/SphereComponent.h"
#include "Curves/CurveVector.h"
#include "Gigantes/GtGameplayTags.h"
#include "Gigantes/Character/GtHeroCharacter.h"
#include "Gigantes/Gameplay/Damage/GtDamageable.h"
#include "Gigantes/Physics/GtCollisionChannels.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"


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

    ConsecutiveShotCount = 0;
    AccumulatedRecoil = FVector2D::ZeroVector;
    CurrentSpread = 0.0f;

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
        GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
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
    // 연사 카운트 증가
    ConsecutiveShotCount++;

    // 연사 카운트 리셋 타이머 재시작 (Single/Auto 모드)
    GetWorld()->GetTimerManager().ClearTimer(ShotCountResetTimer);
    GetWorld()->GetTimerManager().SetTimer(
        ShotCountResetTimer,
        [this]()
        {
            ConsecutiveShotCount = 0;
            UE_LOG(LogTemp, Verbose, TEXT("Shot count reset after timeout"));
        },
        ShotCountResetDelay,
        false
    );
    
    // 커브 기반 반동 및 확산 적용
    ApplyInstantRecoil();
    ApplyInstantSpread();
    
    // 애니메이션 반동 몽타주 재생
    if (FireRecoilMontage)
    {
        if (ACharacter* OwnerCharacter = Cast<ACharacter>(WeaponOwner))
        {
            if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
            {
                if (AnimInstance->Montage_IsPlaying(FireRecoilMontage))
                {
                 AnimInstance->Montage_Stop(0.1f, FireRecoilMontage);
                }

                AnimInstance->Montage_Play(FireRecoilMontage, 1.f);
            }
        }
    }

    PlayMuzzleFlash();
    
    FHitResult HitResult;
    // '2-Trace' 방법으로 최종 목표 지점 결정
    bool bHit = GetTargetHitResult(HitResult);

    if (TrailEffect)
    {
        FVector MuzzleLocation = WeaponMesh->GetSocketLocation(TEXT("MuzzleSocket"));
        FVector EndPoint = bHit ? HitResult.ImpactPoint : HitResult.TraceEnd;
        
        SpawnTrailEffect(MuzzleLocation, EndPoint);
    }
    
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
        if (ImpactEffect)
        {
            // 히트 노말 방향으로 이펙트 회전
            FRotator ImpactRotation = HitResult.ImpactNormal.Rotation();
            
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                ImpactEffect,
                HitResult.ImpactPoint,
                ImpactRotation
            );
        }
        
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
    ConsecutiveShotCount = 0;
    AccumulatedRecoil = FVector2D::ZeroVector;
    CurrentSpread = 0.0f;

    GetWorld()->GetTimerManager().ClearTimer(RecoilRecoveryTimer);
    GetWorld()->GetTimerManager().ClearTimer(ShotCountResetTimer);
    
    UE_LOG(LogTemp, Warning, TEXT("[TestWeapon] Reloaded! Ammo: %d/%d"), CurrentAmmo, TestMaxAmmo);
}

void AGtTestWeaponBase::ApplyInstantRecoil()
{
    if (!RecoilData.RecoilPatternCurve || !WeaponOwner) 
        return;
    
    APawn* OwnerPawn = Cast<APawn>(WeaponOwner);
    APlayerController* PC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
    if (!PC) 
        return;

    // 1. 현재 발사와 이전 발사의 반동 위치를 커브에서 읽음
    const FVector CurrentShotRecoilVector = RecoilData.RecoilPatternCurve->GetVectorValue(ConsecutiveShotCount);
    const FVector PreviousShotRecoilVector = RecoilData.RecoilPatternCurve->GetVectorValue(ConsecutiveShotCount - 1);

    // 2. 두 위치의 차이(Delta)를 계산. 이것이 이번 프레임에 추가할 순수 반동량
    // (ConsecutiveShotCount가 1일 때는 Previous가 0이므로 첫 반동량이 그대로 적용됨)
    const FVector2D RecoilDelta(
        CurrentShotRecoilVector.X - PreviousShotRecoilVector.X, // Pitch Delta
        CurrentShotRecoilVector.Y - PreviousShotRecoilVector.Y  // Yaw Delta
    );
    
    // 3. 조준 상태와 강도에 따라 최종 반동량을 조절
    float Multiplier = bIsAiming ? RecoilData.AimRecoilMultiplier : 1.0f;
    Multiplier *= RecoilData.RecoilIntensity;

    const FVector2D FinalRecoilDelta = RecoilDelta * Multiplier;

    // 4. 계산된 '차이(Delta)'만큼만 컨트롤러에 입력을 추가
    PC->AddPitchInput(FinalRecoilDelta.X);
    PC->AddYawInput(FinalRecoilDelta.Y);
    
    // 5. 전체 누적 반동량을 업데이트. 이 값은 회복 로직에서 사용됨
    AccumulatedRecoil += FinalRecoilDelta;
    
    // 6. 회복 타이머 재시작.
    StartRecoilRecovery();
}

void AGtTestWeaponBase::StartRecoilRecovery()
{
    bRecoilRecoveryStarted = false;
    
    GetWorld()->GetTimerManager().ClearTimer(RecoilRecoveryTimer);
    GetWorld()->GetTimerManager().SetTimer(
        RecoilRecoveryTimer,
        this,
        &AGtTestWeaponBase::ProcessRecoilRecovery,
        RecoilData.RecoveryDelay,
        false
    );
}

void AGtTestWeaponBase::ProcessRecoilRecovery()
{
    if (!WeaponOwner) 
        return;
    
    APawn* OwnerPawn = Cast<APawn>(WeaponOwner);
    APlayerController* PC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
    if (!PC)
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    float ActualDeltaTime;

    // 첫 호출인지 확인
    if (!bRecoilRecoveryStarted)
    {
        // 첫 호출 시 시간 초기화하고 작은 델타 사용
        LastRecoilUpdateTime = CurrentTime;
        bRecoilRecoveryStarted = true;
        ActualDeltaTime = 0.016f;  // 첫 프레임은 60fps 기준으로
    }
    else
    {
        // 이후부터는 실제 경과 시간 사용
        ActualDeltaTime = CurrentTime - LastRecoilUpdateTime;
        LastRecoilUpdateTime = CurrentTime;
    }

    // 델타타임이 너무 크면 제한 (프레임 드롭 대응)
    ActualDeltaTime = FMath::Min(ActualDeltaTime, 0.1f);
    
    const float Alpha = FMath::Clamp(RecoilData.RecoverySpeed * ActualDeltaTime, 0.0f, 1.0f);
    
    // 0으로 회복
    FVector2D NewRecoil = FMath::Lerp(AccumulatedRecoil, FVector2D::ZeroVector, Alpha);
    FVector2D DeltaRecoil = NewRecoil - AccumulatedRecoil;
    
    // 회복 적용
    PC->AddPitchInput(DeltaRecoil.X);
    PC->AddYawInput(DeltaRecoil.Y);
    
    AccumulatedRecoil = NewRecoil;

    // 계속 회복이 필요하면 타이머 재설정
    if (AccumulatedRecoil.SizeSquared() > 0.01f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            RecoilRecoveryTimer,
            this,
            &AGtTestWeaponBase::ProcessRecoilRecovery,
            0.016f,  // 타이머 호출 간격 (목표치일 뿐, 실제 델타는 위에서 계산)
            false
        );
    }
    else
    {
        // 완전 회복
        AccumulatedRecoil = FVector2D::ZeroVector;
        ConsecutiveShotCount = 0;
    }
}

void AGtTestWeaponBase::ApplyInstantSpread()
{
    if (!SpreadData.SpreadCurve) 
        return;
    
    // 커브에서 현재 샷에 해당하는 확산도 가져오기
    float TargetSpread = SpreadData.SpreadCurve->GetFloatValue(ConsecutiveShotCount);
    
    // 조준 중이면 확산 감소
    if (bIsAiming)
    {
        TargetSpread *= SpreadData.AimSpreadMultiplier;
    }
    
    // 최대값 제한
    CurrentSpread = FMath::Min(TargetSpread, SpreadData.MaxSpread);

    // 확산 적용 후 즉시 회복 시작
    StartSpreadRecovery();
}

void AGtTestWeaponBase::StartSpreadRecovery()
{
    GetWorld()->GetTimerManager().ClearTimer(SpreadRecoveryTimer);
    GetWorld()->GetTimerManager().SetTimer(
        SpreadRecoveryTimer,
        this,
        &AGtTestWeaponBase::ProcessSpreadRecovery,
        0.1f,
        true
    );
}

void AGtTestWeaponBase::ProcessSpreadRecovery()
{
    // 점진적 회복
    CurrentSpread = FMath::Max(0.0f, CurrentSpread - SpreadData.SpreadRecoveryRate * 0.1f);
    
    // 완전 회복 시 타이머 정지
    if (CurrentSpread <= 0.0f)
    {
        GetWorld()->GetTimerManager().ClearTimer(SpreadRecoveryTimer);
        CurrentSpread = 0.0f;
    }
}

bool AGtTestWeaponBase::GetTargetHitResult(FHitResult& OutHitResult) const
{
    APawn* OwnerPawn = Cast<APawn>(WeaponOwner);
    APlayerController* PC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
    if (!PC) 
        return false;

    // 1. 카메라 시점 정보 획득
    FVector CamLoc;
    FRotator CamRot;
    PC->GetPlayerViewPoint(CamLoc, CamRot);
    const FVector BaseAimDir = CamRot.Vector().GetSafeNormal();

    // 2. 확산(Spread) 적용
    const float SpreadAngleRad = FMath::DegreesToRadians(CurrentSpread);
    const FVector FinalAimDir = FMath::VRandCone(BaseAimDir, SpreadAngleRad);

    // 3. 3인칭 시차 보정을 위한 투영 계산
    const double FocalDistance = 1024.0f;
    const FVector FocalLoc = CamLoc + (FinalAimDir * FocalDistance);
    const FVector PawnLoc = OwnerPawn->GetActorLocation();
    const FVector ProjectedStartLoc = FocalLoc + (((PawnLoc - FocalLoc) | FinalAimDir) * FinalAimDir);

    // 4. 1차 트레이스: 투영된 위치에서 조준 방향으로
    const FVector TraceEnd = ProjectedStartLoc + (FinalAimDir * 10000.0f);
    FHitResult CameraTraceHit;
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(WeaponOwner);
    QueryParams.AddIgnoredActor(this);

    bool bCameraHit = GetWorld()->LineTraceSingleByChannel(
        CameraTraceHit, 
        ProjectedStartLoc,  // 카메라 위치가 아닌 투영된 위치에서 시작
        TraceEnd, 
        Gt_TraceChannel_Weapon_Capsule, 
        QueryParams);

    // 5. 목표 지점 결정
    FVector TargetLocation = bCameraHit ? CameraTraceHit.ImpactPoint : TraceEnd;

    // 6. 총구 위치 가져오기
    const FVector MuzzleLocation = WeaponMesh->GetSocketLocation(TEXT("MuzzleSocket"));
    
    // 7. 총구에서 목표 지점까지의 방향 계산
    const FVector MuzzleToTarget = (TargetLocation - MuzzleLocation).GetSafeNormal();
    
    // 8. 2차 트레이스: 총구에서 목표 방향으로 발사
    const FVector MuzzleTraceEnd = MuzzleLocation + (MuzzleToTarget * 10000.0f);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        OutHitResult, 
        MuzzleLocation, 
        MuzzleTraceEnd, 
        Gt_TraceChannel_Weapon_Capsule, 
        QueryParams);

    // 디버그 표시 (선택사항)
    if (bShowDebugLine)
    {
        // 1차 트레이스 (투영 위치 → 타겟) - 파란색, 얇게
        DrawDebugLine(GetWorld(), ProjectedStartLoc, TargetLocation,
            FColor::Blue, false, DebugLineDuration * 0.5f, 0, 1.0f);
        
        // 총구 위치 표시 - 주황색
        DrawDebugSphere(GetWorld(), MuzzleLocation,
            5.0f, 8, FColor::Orange, false, DebugLineDuration);
    }
    
    return bHit;
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

void AGtTestWeaponBase::PlayMuzzleFlash()
{
    if (!MuzzleFlashEffect)
        return;
    
    // 컴포넌트가 없으면 처음 한 번만 생성
    if (!MuzzleFlashComponent)
    {
        MuzzleFlashComponent = UGameplayStatics::SpawnEmitterAttached(
            MuzzleFlashEffect,
            WeaponMesh,
            TEXT("MuzzleSocket"),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            FVector(MuzzleFlashScale),
            EAttachLocation::SnapToTarget,
            false  // Auto Destroy 끄기
        );
        
        if (MuzzleFlashComponent)
        {
            MuzzleFlashComponent->bAutoDestroy = false;
            MuzzleFlashComponent->SetAutoActivate(false);
        }
    }
    
    // 이펙트 재생
    if (MuzzleFlashComponent)
    {
        MuzzleFlashComponent->Activate(true);
        
        // WeakObjectPtr를 사용한 안전한 타이머
        TWeakObjectPtr<AGtTestWeaponBase> WeakThis(this);
        
        GetWorld()->GetTimerManager().ClearTimer(MuzzleFlashOffTimer);
        GetWorld()->GetTimerManager().SetTimer(
            MuzzleFlashOffTimer,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->StopMuzzleFlash();
                }
            },
            MuzzleFlashDuration,
            false
        );
    }
}

void AGtTestWeaponBase::StopMuzzleFlash()
{
    if (MuzzleFlashComponent)
    {
        MuzzleFlashComponent->Deactivate();
    }
}

void AGtTestWeaponBase::SpawnTrailEffect(const FVector& Origin, const FVector& Impact)
{
    if (!TrailEffect)
        return;
    
    UParticleSystemComponent* TrailComp = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        TrailEffect,
        Origin,
        FRotator::ZeroRotator,
        FVector(1.0f),
        true  // Auto Destroy
    );
    
    if (TrailComp)
    {
        // Beam의 타겟 위치 설정
        TrailComp->SetVectorParameter(TrailTargetParameterName, Impact);
    }
}