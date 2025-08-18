#include "GtHandGun.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "DrawDebugHelpers.h"

AGtHandGun::AGtHandGun()
{
	// 에디터 프리뷰용 기본값(실제 게임에선 InitFromData로 덮어쓰기)
	ItemData.Damage        = 25;
	ItemData.FireRate      = 0.2f;
	ItemData.ReloadTime    = 1.2f;
	ItemData.MaxAmmo       = 12;
	ItemData.AmmoInMagazine= 12;
}

void AGtHandGun::InitFromData(const FGtItemData& InData)
{
	Super::InitFromData(InData);
	// 필요하면 권총 전용 파라미터도 여기서 데이터로 받도록 확장 가능
}

void AGtHandGun::OnFire_Implementation()
{
	// HandGun: 단발 히트스캔
	this->Damage = (float)ItemData.Damage;
	DoHitscanShot(Damage, DefaultRange, DefaultSpreadDeg);
}

bool AGtHandGun::DoHitscanShot(float InDamage, float InRange, float InSpreadDeg)
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

	// 간단한 랜덤 스프레드(원뿔)
	const float SpreadRad = FMath::DegreesToRadians(InSpreadDeg);
	const FVector ShootDir = FMath::VRandCone(CamRot.Vector(), SpreadRad);
	const FVector TraceStart = CamLoc;
	const FVector TraceEnd   = TraceStart + ShootDir * InRange;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(HandGunShot), true, this);
	Params.AddIgnoredActor(this);
	if (Inst) Params.AddIgnoredActor(Inst);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

#if ENABLE_DRAW_DEBUG
	// 디버그 라인
	DrawDebugLine(GetWorld(), TraceStart, bHit ? Hit.ImpactPoint : TraceEnd, FColor::Green, false, 1.5f, 0, 0.8f);
	if (bHit) DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 8.f, FColor::Red, false, 2.f);
#endif

	if (bHit && Hit.GetActor())
	{
		float FinalDamage = InDamage;

		// 간단 헤드샷 판정(본 이름 기준)
		if (Hit.BoneName == FName("head") || Hit.BoneName == FName("Head"))
		{
			FinalDamage *= HeadshotMultiplier;
		}

		UGameplayStatics::ApplyPointDamage(
			Hit.GetActor(),
			FinalDamage,
			ShootDir,
			Hit,
			Inst ? Inst->GetController() : nullptr,
			this,
			DamageTypeClass ? DamageTypeClass.Get() : UDamageType::StaticClass()
		);
	}

	// Muzzle FX/사운드는 부모 OnFire에서 이미 처리했다면 생략 가능(필요 시 여기서 추가)
	return true;
}