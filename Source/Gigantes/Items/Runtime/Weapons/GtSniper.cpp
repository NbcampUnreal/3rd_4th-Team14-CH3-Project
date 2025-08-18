#include "GtSniper.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "DrawDebugHelpers.h"

AGtSniper::AGtSniper()
{
	// 에디터 프리뷰용 기본값
	ItemData.Damage         = 90;
	ItemData.FireRate       = 1.2f;     // 볼트액션 느낌
	ItemData.ReloadTime     = 2.5f;
	ItemData.MaxAmmo        = 5;
	ItemData.AmmoInMagazine = 5;
}

void AGtSniper::InitFromData(const FGtItemData& InData)
{
	Super::InitFromData(InData);
}

void AGtSniper::OnFire_Implementation()
{
	// 간단하게: 발사 순간 잠깐 줌 걸었다가 바로 되돌리기(원하면 입력 이벤트로 분리)
	if (bUseADSZoom) ApplyADS(true);

	this->Damage = (float)ItemData.Damage;
	DoHitscanShot(Damage, DefaultRange, DefaultSpreadDeg);

	if (bUseADSZoom) ApplyADS(false);
}

void AGtSniper::ApplyADS(bool bEnable)
{
	APawn* Inst = GetInstigator();
	APlayerController* PC = Inst ? Cast<APlayerController>(Inst->GetController()) : nullptr;
	if (!PC || !PC->PlayerCameraManager) return;

	PC->PlayerCameraManager->SetFOV(bEnable ? ADSFOV : ADSRestoreFOV);
}

bool AGtSniper::DoHitscanShot(float InDamage, float InRange, float InSpreadDeg)
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
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SniperShot), true, this);
	Params.AddIgnoredActor(this);
	if (Inst) Params.AddIgnoredActor(Inst);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

#if ENABLE_DRAW_DEBUG
	DrawDebugLine(GetWorld(), TraceStart, bHit ? Hit.ImpactPoint : TraceEnd, FColor::Blue, false, 2.f, 0, 1.0f);
	if (bHit) DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 10.f, FColor::Cyan, false, 2.f);
#endif

	if (bHit && Hit.GetActor())
	{
		float FinalDamage = InDamage;

		// 헤드샷 가중
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

	return true;
}