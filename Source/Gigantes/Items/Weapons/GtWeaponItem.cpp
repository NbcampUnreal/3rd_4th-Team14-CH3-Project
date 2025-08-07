#include "GtWeaponItem.h"


// Sets default values
AGtWeaponItem::AGtWeaponItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AGtWeaponItem::InitFromData(const FGtItemData& InData)
{
	Super::InitFromData(InData);
	ItemData.Damage = InData.Damage;
	ItemData.FireRate = InData.FireRate;
	ItemData.ReloadTime = InData.ReloadTime;
	ItemData.MaxAmmo = InData.MaxAmmo;
	ItemData.AmmoInMagazine = InData.AmmoInMagazine;

	// UE_LOG(LogTemp, Log, TEXT("[Init] AK-47 Damage: %d / Rate: %f / MaxAmmo: %d"), Damage, FireRate, MaxAmmo);

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

	if (GetWorld()->GetTimerManager().IsTimerActive(FireRateHandle))
	{
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[Riffle] Fire! Rate: %f"), FireRate);

	--AmmoInMagazine;

	GetWorld()->GetTimerManager().SetTimer(
		FireRateHandle,
		this,
		&AGtWeaponItem::ResetFireCooldown,
		FireRate,
		false
		);

}

void AGtWeaponItem::Reload()
{
	if (bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("Already Reload"));
		return;
	}

	if (AmmoInMagazine == MaxAmmo)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Riffle] Magazine full"));
		return;
	}
	
	bIsReloading = true;
	UE_LOG(LogTemp, Warning, TEXT("[Riffle] Reloading..."));

	GetWorld()->GetTimerManager().SetTimer(ReloadTimer, [this]()
	{
		AmmoInMagazine = MaxAmmo;
		bIsReloading = false;
		UE_LOG(LogTemp, Warning, TEXT("[Riffle] Realod Complete"));
	}, ReloadRate, false);
}

void AGtWeaponItem::OnFire()
{
	Fire();
}

void AGtWeaponItem::OnReload()
{
	Reload();
}

void AGtWeaponItem::ResetFireCooldown()
{
	GetWorld()->GetTimerManager().ClearTimer(FireRateHandle);
}
