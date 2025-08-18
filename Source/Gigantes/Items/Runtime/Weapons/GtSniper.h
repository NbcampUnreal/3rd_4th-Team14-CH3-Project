#pragma once
#include "CoreMinimal.h"
#include "Gigantes/Items/Runtime/Weapons/GtWeaponItem.h"
#include "GtSniper.generated.h"

UCLASS()
class GIGANTES_API AGtSniper : public AGtWeaponItem
{
	GENERATED_BODY()
public:
	AGtSniper();

	virtual void InitFromData(const FGtItemData& InData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Sniper")
	float DefaultRange = 100000.f;               // 1km

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Sniper")
	float DefaultSpreadDeg = 0.1f;               // 사실상 없음

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Sniper")
	float HeadshotMultiplier = 2.5f;

	/** 조준(ADS)용: 발사 시 FOV 축소 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Sniper|ADS")
	bool bUseADSZoom = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Sniper|ADS", meta=(EditCondition="bUseADSZoom"))
	float ADSFOV = 30.f;                         // 스나이핑 시 FOV

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Sniper|ADS", meta=(EditCondition="bUseADSZoom"))
	float ADSRestoreFOV = 90.f;                  // 복귀 FOV(프로젝트 기본 FOV에 맞춰 조정)

	virtual void OnFire_Implementation() override;

	/** 필요 시 Scope In/Out을 캐릭터 입력과 연결해서 따로 호출해도 됨 */
	void ApplyADS(bool bEnable);

private:
	bool DoHitscanShot(float InDamage, float InRange, float InSpreadDeg);
};