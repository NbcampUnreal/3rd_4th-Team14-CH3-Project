#pragma once
#include "CoreMinimal.h"
#include "Gigantes/Items/Runtime/Weapons/GtWeaponItem.h"
#include "GtHandGun.generated.h"

UCLASS()
class GIGANTES_API AGtHandGun : public AGtWeaponItem
{
	GENERATED_BODY()
public:
	AGtHandGun();

	/** 데이터 덮어쓰기: FGtItemData (Damage, FireRate, ReloadTime, Ammo 등) */
	virtual void InitFromData(const FGtItemData& InData) override;

protected:
	/** 권총 기본 파라미터(데이터로 덮어쓰기 가능) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|HandGun")
	float DefaultRange = 20000.f;            // 약 200m

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|HandGun")
	float DefaultSpreadDeg = 1.5f;           // 기본 탄퍼짐

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|HandGun")
	float HeadshotMultiplier = 2.0f;

	/** 실제 발사(부모 Fire가 호출 → OnFire_Implementation로 들어옴) */
	virtual void OnFire_Implementation() override;

private:
	bool DoHitscanShot(float InDamage, float InRange, float InSpreadDeg);
};