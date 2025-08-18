#include "GtRifle.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "DrawDebugHelpers.h"

AGtRifle::AGtRifle()
{
    PrimaryActorTick.bCanEverTick = true; // 스프레드 회복용

    // 에디터 프리뷰용 기본값(실제 게임은 InitFromData로 덮어쓰기 추천)
    ItemData.Damage         = 30;
    ItemData.FireRate       = 0.1f;  // 연사 간격(초) = 600RPM
    ItemData.ReloadTime     = 1.9f;
    ItemData.MaxAmmo        = 30;
    ItemData.AmmoInMagazine = 30;
}

void AGtRifle::InitFromData(const FGtItemData& InData)
{
    Super::InitFromData(InData);
    // 필요 시 Rifle 전용 파라미터를 FGtItemData에 확장해서 여기서 덮어쓰기
}

void AGtRifle::BeginPlay()
{
    Super::BeginPlay();
    CurrentSpreadDeg = BaseSpreadDeg;
}

void AGtRifle::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // 스프레드 회복: 발사 중에도 너무 벌어지지 않도록 서서히 복원
    const float TargetSpread = BaseSpreadDeg;
    if (CurrentSpreadDeg > TargetSpread)
    {
        CurrentSpreadDeg = FMath::Max(TargetSpread, CurrentSpreadDeg - SpreadRecoveryPerSec * DeltaSeconds);
    }
    else
    {
        CurrentSpreadDeg = FMath::Min(TargetSpread, CurrentSpreadDeg + SpreadRecoveryPerSec * 0.3f * DeltaSeconds); // 너무 좁지 않게
    }
}

void AGtRifle::StartTrigger()
{
    bHoldingTrigger = true;

    switch (FireMode)
    {
        case ERifleFireMode::Semi:
        {
            // 반자동: 한 번만 발사(부모 Fire가 발사간격 체크/탄약 체크 한다고 가정)
            Fire();
            break;
        }
        case ERifleFireMode::Burst:
        {
            if (!GetWorldTimerManager().IsTimerActive(BurstTimer))
            {
                ShotsLeftInBurst = BurstCount;
                GetWorldTimerManager().SetTimer(BurstTimer, this, &AGtRifle::BurstFireTick, BurstInterval, true, 0.f);
            }
            break;
        }
        case ERifleFireMode::Auto:
        {
            if (!GetWorldTimerManager().IsTimerActive(AutoTimer))
            {
                const float Interval = FMath::Max(0.01f, ItemData.FireRate);
                GetWorldTimerManager().SetTimer(AutoTimer, this, &AGtRifle::AutoFireTick, Interval, true, 0.f);
            }
            break;
        }
    }

    if (bUseADSZoom) ApplyADS(true);
}

void AGtRifle::ReleaseTrigger()
{
    bHoldingTrigger = false;

    if (FireMode == ERifleFireMode::Auto)
    {
        GetWorldTimerManager().ClearTimer(AutoTimer);
    }
    // 버스트는 ShotsLeftInBurst가 0 될 때까지 진행(일반적인 구현)
    if (bUseADSZoom) ApplyADS(false);
}

void AGtRifle::ToggleFireMode()
{
    switch (FireMode)
    {
        case ERifleFireMode::Auto:  FireMode = ERifleFireMode::Burst; break;
        case ERifleFireMode::Burst: FireMode = ERifleFireMode::Semi;  break;
        case ERifleFireMode::Semi:  FireMode = ERifleFireMode::Auto;  break;
    }

    // 트리거 유지 중 모드가 바뀌면 루프 재설정
    if (bHoldingTrigger)
    {
        ReleaseTrigger();
        StartTrigger();
    }
}

void AGtRifle::AutoFireTick()
{
    // 부모 Fire()가 연사 간격/탄약 체크 → OnFire 호출한다고 가정
    Fire();

    // 스프레드 누적
    CurrentSpreadDeg = FMath::Min(CurrentSpreadDeg + SpreadPerShotDeg, MaxSpreadDeg);
}

void AGtRifle::BurstFireTick()
{
    if (ShotsLeftInBurst <= 0)
    {
        GetWorldTimerManager().ClearTimer(BurstTimer);
        return;
    }

    Fire();
    ShotsLeftInBurst--;

    CurrentSpreadDeg = FMath::Min(CurrentSpreadDeg + SpreadPerShotDeg, MaxSpreadDeg);
}

void AGtRifle::OnFire_Implementation()
{
    this->Damage = (float)ItemData.Damage;
    const float Range = 30000.f; // 300m 기본 사거리(필요하면 EditDefaultsOnly로 승격)
    DoHitscanShot(Damage, Range, CurrentSpreadDeg);
    ApplyRecoil();
}

bool AGtRifle::DoHitscanShot(float InDamage, float InRange, float InSpreadDeg)
{
    APawn* Inst = GetInstigator();
    APlayerController* PC = Inst ? Cast<APlayerController>(Inst->GetController()) : nullptr;

    FVector CamLoc; FRotator CamRot;
    if (PC && PC->PlayerCameraManager)
    {
        CamLoc = PC->PlayerCameraManager->GetCameraLocation();
        CamRot = PC->PlayerCameraManager->GetCameraRotation();
    }
    else
    {
        CamLoc = GetActorLocation();
        CamRot = GetActorRotation();
    }

    const float SpreadRad = FMath::DegreesToRadians(InSpreadDeg);
    const FVector ShootDir = FMath::VRandCone(CamRot.Vector(), SpreadRad);
    const FVector TraceStart = CamLoc;
    const FVector TraceEnd   = TraceStart + ShootDir * InRange;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(RifleShot), true, this);
    Params.AddIgnoredActor(this);
    if (Inst) Params.AddIgnoredActor(Inst);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

#if ENABLE_DRAW_DEBUG
    DrawDebugLine(GetWorld(), TraceStart, bHit ? Hit.ImpactPoint : TraceEnd, FColor::Yellow, false, 1.2f, 0, 0.8f);
    if (bHit) DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 8.f, FColor::Orange, false, 1.5f);
#endif

    if (bHit && Hit.GetActor())
    {
        UGameplayStatics::ApplyPointDamage(
            Hit.GetActor(),
            InDamage,
            ShootDir,
            Hit,
            Inst ? Inst->GetController() : nullptr,
            this,
            DamageTypeClass ? DamageTypeClass.Get() : UDamageType::StaticClass()
        );
    }
    return true;
}

void AGtRifle::ApplyADS(bool bEnable)
{
    if (!bUseADSZoom) return;

    APawn* Inst = GetInstigator();
    APlayerController* PC = Inst ? Cast<APlayerController>(Inst->GetController()) : nullptr;
    if (!PC || !PC->PlayerCameraManager) return;

    PC->PlayerCameraManager->SetFOV(bEnable ? ADSFOV : ADSRestoreFOV);
}

void AGtRifle::ApplyRecoil()
{
    APawn* Inst = GetInstigator();
    APlayerController* PC = Inst ? Cast<APlayerController>(Inst->GetController()) : nullptr;
    if (!PC || !PC->PlayerCameraManager) return;

    const float PitchKick = RecoilPitchPerShot; // 음수(위로 차오름)
    const float YawKick = FMath::FRandRange(-RecoilYawJitterPerShot, RecoilYawJitterPerShot);

    FRotator NewRot = PC->PlayerCameraManager->GetCameraRotation();
    NewRot.Pitch += PitchKick;
    NewRot.Yaw   += YawKick;

    PC->SetControlRotation(NewRot);
}