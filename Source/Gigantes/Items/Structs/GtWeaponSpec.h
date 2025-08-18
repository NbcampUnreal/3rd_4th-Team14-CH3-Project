#pragma once
#include "CoreMinimal.h"
#include "GtWeaponSpec.generated.h"

USTRUCT(BlueprintType)
struct FGtWeaponSpec {
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Damage = 25.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float FireRate = 600.f; // RPM
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ReloadTime = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxAmmo = 30;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 AmmoInMagazine = 30;
};