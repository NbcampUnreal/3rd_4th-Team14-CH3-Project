#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "Gigantes/Items/Runtime/Core/GtItemBase.h"
#include "ItemClassMapping.generated.h"

USTRUCT()
struct FItemClassMappingRow
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FGameplayTag ItemTag;
	
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<AGtItemBase> ItemClass;
};

UCLASS(BlueprintType)
class GIGANTES_API UItemClassMapping : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Mappings")
	TArray<FItemClassMappingRow> Mappings;
};