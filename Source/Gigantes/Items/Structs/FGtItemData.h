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
	int32 Damage = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AmmoInMagazine = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireRate = 0.1f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReloadTime = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExplosionRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExplosionDelay = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HealAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ItemTag;
};
