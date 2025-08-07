#include "GtItemManagerComponent.h"

#include "GtItemFactory.h"

// Sets default values for this component's properties
UGtItemManagerComponent::UGtItemManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

/*
 * 아이템 데이터 로드
 */
void UGtItemManagerComponent::LoadAllItemData()
{
	FString Path = FPaths::ProjectContentDir() / TEXT("Data/ItemData.json");

	if (UGtItemFactory::LoadItemData(Path, ItemDataMap))
	{
		UE_LOG(LogTemp, Log, TEXT("Item data loaded: %d items"), ItemDataMap.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load item data from: %s"), *Path);
	}
}

/*
 * 아이템 장착
 */
void UGtItemManagerComponent::GiveItemToPlayer(const FString& ItemId)
{
	if (!ItemDataMap.Contains(ItemId)) return;

	const FGtItemData& Data = ItemDataMap[ItemId];
	AGtItemBase* NewItem = UGtItemFactory::CreateItem(Data, GetWorld());

	if (NewItem)
	{
		Inventory.Add(NewItem);
		UE_LOG(LogTemp, Log, TEXT("Created and added item: %s"), *Data.ItemName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create item: %s"), *ItemId);
	}
}

/*
 * 아이템 사용
 */
void UGtItemManagerComponent::UseItem(int32 Index)
{
	if (Inventory.IsValidIndex(Index))
	{
		Inventory[Index]->UseItem();
	}
}