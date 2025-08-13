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
	TArray<FString> Dirs;
    
	// 에디터에서 DataDirectories가 설정 ? 기본 경로를 사용
	if (DataDirectories.Num() == 0)
	{
		Dirs = {
			FPaths::ProjectContentDir() / TEXT("Items/Data"),
			FPaths::ProjectContentDir() / TEXT("Data/Weapons") // 필요에 따라 추가
		 };
	}
	else
	{
		for (const auto& D : DataDirectories)
		{
			Dirs.Add(D.Path);
		}
	}

	ItemDataMap.Reset();
	const int32 Files = UGtItemFactory::LoadItemDataFromDirs(Dirs, ItemDataMap);
	UE_LOG(LogTemp, Log, TEXT("[ItemManager] Loaded %d files, %d items"), Files, ItemDataMap.Num());
}

/*
 * 아이템 장착
 */
void UGtItemManagerComponent::GiveItemToPlayer(const FString& ItemId)
{
	if (!ItemDataMap.Contains(ItemId))
	{
		UE_LOG(LogTemp, Error, TEXT("Item ID not found in map: %s"), *ItemId);
		return;
	}

	const FGtItemData& Data = ItemDataMap[ItemId];
	AGtItemBase* NewItem = UGtItemFactory::CreateItem(Data, GetWorld());

	if (NewItem)
	{
		const AActor* OwnerActor = GetOwner();
		const FVector Base = OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector;
		NewItem->SetActorLocation(Base + FVector(100.f, 0.f, 0.f));
       
		// Inventory는 헤더에 UPROPERTY로 선언된 TArray<AGtItemBase*>라고 가정합니다.
		// NewItem->AttachToComponent(OwnerActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
		// 인벤토리 로직에 맞게 아이템을 추가합니다.
		// Inventory.Add(NewItem);
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