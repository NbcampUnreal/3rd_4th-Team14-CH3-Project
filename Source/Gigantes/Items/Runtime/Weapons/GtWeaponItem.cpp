#include "GtWeaponItem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"


// Sets default values
AGtWeaponItem::AGtWeaponItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(Root);

	// (옵션) 재활용형 이펙트 쓸 때만 에디터에서 할당해서 사용
	MuzzleFlashComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MuzzleFlashComponent"));
	MuzzleFlashComponent->SetupAttachment(WeaponMesh);
	MuzzleFlashComponent->bAutoActivate = false;
}

void AGtWeaponItem::InitFromData(const FGtItemData& InData)
{
	Super::InitFromData(InData);
	if (InData.Damage       > 0.f) Damage        = InData.Damage;
	if (InData.FireRate     > 0.f) FireRate  = InData.FireRate;
	if (InData.ReloadTime   > 0.f) ReloadTime    = InData.ReloadTime;
	if (InData.MaxAmmo      > 0)   MaxAmmo       = InData.MaxAmmo;
	if (InData.AmmoInMagazine >= 0) AmmoInMagazine = FMath::Clamp(InData.AmmoInMagazine, 0, MaxAmmo);

}
bool AGtWeaponItem::IsOnFireCooldown() const
{
	if (const UWorld* World = GetWorld())
		return World->GetTimerManager().IsTimerActive(FireCooldownHandle);
	return false;
}

void AGtWeaponItem::StartFireCooldown()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FireCooldownHandle,
			/*InDelegate=*/FTimerDelegate(),
			/*InRate=*/FMath::Max(0.f, FireRate),
			/*InbLoop=*/false
		);
	}
}

void AGtWeaponItem::Fire()
{
	if (bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot Fire"));
		return;
	}

	if (AmmoInMagazine <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Ammo, Reload Please"));
		return;
	}

	if (IsOnFireCooldown()) return;
	
	UE_LOG(LogTemp, Warning, TEXT("[Riffle] Fire! Rate: %f"), FireRate);

	--AmmoInMagazine;

	PlayMuzzleFlash();
	OnFire();

	DoHitscan();
	
	StartFireCooldown();
}

void AGtWeaponItem::Reload()
{
	if (bIsReloading) return;
	if (AmmoInMagazine >= MaxAmmo) return;
	
	bIsReloading = true;
	OnReload();
	UE_LOG(LogTemp, Warning, TEXT("[Riffle] Reloading..."));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ReloadTimerHandle, [this]()
		{
			AmmoInMagazine = MaxAmmo;
			bIsReloading = false;
			UE_LOG(LogTemp, Warning, TEXT("[Riffle] Realod Complete"));
		},  FMath::Max(0.f, ReloadTime), false);
	}
}

void AGtWeaponItem::DoHitscan()
{
	if (!GetWorld()) return;

	const FVector Origin = GetShootOrigin();
	const FVector Dir    = GetShootDirection();
	const FVector End    = Origin + Dir * TraceRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponHitscan), true);
	Params.AddIgnoredActor(this);
	if (AActor* OwnerActor = GetOwner()) Params.AddIgnoredActor(OwnerActor);

	FHitResult Hit;
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Origin, End, HitScanChannel, Params);

#if ENABLE_DRAW_DEBUG
	DrawDebugLine(GetWorld(), Origin, bHit ? Hit.ImpactPoint : End, FColor::Green, false, 1.0f, 0, 1.0f);
	if (bHit) DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 8.f, FColor::Red, false, 1.0f);
#endif

	if (!bHit) return;

	AActor* HitActor = Hit.GetActor();

	TSubclassOf<UDamageType> FinalDamageType =
	(DamageTypeClass != nullptr)
		? DamageTypeClass
		: TSubclassOf<UDamageType>(UDamageType::StaticClass());
	
	if (HitActor)
	{
		AController* InstigatorCtrl = GetOwnerController();
		UGameplayStatics::ApplyPointDamage(
			HitActor,
			Damage,
			Dir,
			Hit,
			InstigatorCtrl,
			this,
			FinalDamageType
		);
	}

	OnHit(Hit);
}

void AGtWeaponItem::PlayMuzzleFlash()
{
	if (!WeaponMesh) return;

	const FTransform SocketTransform  = GetMuzzleSocketTransform();

	// 권장: 소켓에 “원샷”으로 붙여 스폰
	if (MuzzleFlashEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			MuzzleFlashEffect,
			WeaponMesh,
			WeaponMesh->DoesSocketExist(MuzzleSocketName) ? MuzzleSocketName : NAME_None,
			SocketTransform .GetLocation(),
			SocketTransform .Rotator(),
			EAttachLocation::KeepWorldPosition,
			/*bAutoDestroy=*/true
		);
	}

	// (옵션) 재활용형을 쓰고 싶을 때:
	// if (MuzzleFlashComponent)
	// {
	//     MuzzleFlashComponent->SetWorldTransform(SocketXform);
	//     MuzzleFlashComponent->Activate(true);
	// }
}

FTransform AGtWeaponItem::GetMuzzleSocketTransform() const
{
	if (WeaponMesh && WeaponMesh->DoesSocketExist(MuzzleSocketName))
	{
		return WeaponMesh->GetSocketTransform(MuzzleSocketName, ERelativeTransformSpace::RTS_World);
	}
	return WeaponMesh ? WeaponMesh->GetComponentTransform() : FTransform::Identity;
}

FVector AGtWeaponItem::GetShootOrigin() const
{
	return GetMuzzleSocketTransform().GetLocation();
}

FVector AGtWeaponItem::GetShootDirection() const
{
	FVector Forward = GetMuzzleSocketTransform().GetRotation().GetForwardVector();

	if (BulletSpreadDeg <= KINDA_SMALL_NUMBER)
		return Forward;

	float SpreadRad = FMath::DegreesToRadians(BulletSpreadDeg);
	return FMath::VRandCone(Forward, SpreadRad).GetSafeNormal();
}

AController* AGtWeaponItem::GetOwnerController() const
{
	if (const APawn* P = Cast<APawn>(GetOwner()))
		return P->GetController();
	return nullptr;
}

void AGtWeaponItem::OnFire_Implementation()
{
	// 기본 동작: 머즐 플래시 정도만 (원하면 소리/리코일도)
	PlayMuzzleFlash();
	UE_LOG(LogTemp, Verbose, TEXT("AGtWeaponItem::OnFire default impl"));
}

void AGtWeaponItem::OnReload_Implementation()
{
	UE_LOG(LogTemp, Verbose, TEXT("AGtWeaponItem::OnReload default impl"));
}