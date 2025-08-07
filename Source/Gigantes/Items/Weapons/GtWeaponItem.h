#pragma once

#include "CoreMinimal.h"
#include "Gigantes/Items/Base/GtItembase.h"
#include "GtWeaponItem.generated.h"

UCLASS()
class GIGANTES_API AGtWeaponItem : public AGtItemBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGtWeaponItem();

	virtual void Fire();
	virtual void Reload();

protected:
	virtual void InitFromData(const FGtItemData& data) override;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float Damage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float FireRate;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ReloadRate;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	int32 MaxAmmo;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	int32 AmmoInMagazion;
	
	bool bIsReloading;
	FTimerHandle ReloadTimer;
};
