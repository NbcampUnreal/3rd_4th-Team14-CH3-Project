#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GtItemFactory.generated.h"

class AGtItemBase;
struct FGtItemData;

UCLASS()
class GIGANTES_API UGtItemFactory : public UObject
{
	GENERATED_BODY()

public:
	// 아이템 스폰 (ID 기반)
	static AActor* SpawnItemById(UWorld* World, const FString& ItemId, const FTransform& SpawnTransform);

	// 아이템 스폰 (데이터 직접 전달)
	static AGtItemBase* SpawnItem(UWorld* World, const FGtItemData& ItemData, const FTransform& SpawnTransform);
};