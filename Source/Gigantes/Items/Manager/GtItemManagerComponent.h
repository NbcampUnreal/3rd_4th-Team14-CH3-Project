#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gigantes/Items/Base/GtItemBase.h"
#include "Gigantes/Items/Structs/FGtItemData.h"
#include "GtItemManagerComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GIGANTES_API UGtItemManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGtItemManagerComponent();

	UFUNCTION(Blueprintable)
	void LoadAllItemData();
	void GiveItemToPlayer(const FString& ItemId);
	void UseItem(int32 Index);

private:
	TMap<FString, FGtItemData> ItemDataMap;
	TArray<AGtItemBase*> Inventory;
	
};
