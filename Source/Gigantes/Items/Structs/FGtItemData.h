#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FGtItemData.generated.h"

USTRUCT(BlueprintType)
struct FGtItemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SubType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Damage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AmmoInMagazine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAmmo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReloadTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExplosionRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HealAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ItemTag;
};
