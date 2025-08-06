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
	ItemData.Damage = ItemData.Damage;
	ItemData.FireRate = ItemData.FireRate;
	ItemData.ReloadTime = ItemData.ReloadTime;
	ItemData.MaxAmmo = ItemData.MaxAmmo;
	ItemData.AmmoInMagazine = ItemData.AmmoInMagazine;
	
}



