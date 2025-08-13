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

	UE_LOG(LogTemp, Warning, TEXT("[ItemManager] DataDirectories.Num=%d"), DataDirectories.Num());
	for (int32 i=0;i<DataDirectories.Num();++i)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemManager]  Dir[%d]=%s"), i, *DataDirectories[i].Path);
	}

	UE_LOG(LogTemp, Warning, TEXT("[ItemManager] LoadAllItemData()"));

	TArray<FString> Dirs;

	auto ToFsDir = [](const FString& In)->FString
	{
		// 에디터에서 /Game/Data/Items 처럼 넣었을 때 실제 Content 폴더로 맵핑
		static const FString GamePrefix = TEXT("/Game/");
		if (In.StartsWith(GamePrefix))
		{
			// "/Game/" 이후 경로를 ProjectContentDir 뒤에 붙인다
			return FPaths::ProjectContentDir() / In.Mid(GamePrefix.Len());
		}
		return In; // 이미 절대경로면 그대로 사용
	};

	if (DataDirectories.Num() == 0)
	{
		Dirs = {
			FPaths::ProjectContentDir() / TEXT("Items/Data"),
			FPaths::ProjectContentDir() / TEXT("Items/Weapons"),
		};
	}
	else
	{
		for (const auto& D : DataDirectories)
		{
			Dirs.Add(ToFsDir(D.Path));
		}
	}

	// 어떤 경로를 읽을 건지 먼저 찍기
	for (const FString& P : Dirs)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemManager] Scan Dir: %s"), *P);
	}

	ItemDataMap.Reset();
	const int32 Files = UGtItemFactory::LoadItemDataFromDirs(Dirs, ItemDataMap);
	UE_LOG(LogTemp, Warning, TEXT("[ItemManager] Loaded %d files, %d items"), Files, ItemDataMap.Num());

	if (ItemDataMap.Num() > 0)
	{
		auto It = ItemDataMap.CreateConstIterator();
		UE_LOG(LogTemp, Warning, TEXT("[ItemManager] First item => %s (%s)"),
			*It->Value.ItemName, *It->Key);
	}
	
	// TArray<FString> Dirs;
 //    
	// // 에디터에서 DataDirectories가 설정 ? 기본 경로를 사용
	// if (DataDirectories.Num() == 0)
	// {
	// 	Dirs = {
	// 		FPaths::ProjectContentDir() / TEXT("Data/Items"),
	// 		FPaths::ProjectContentDir() / TEXT("Data/Weapons") // 필요에 따라 추가
	// 	 };
	// }
	// else
	// {
	// 	for (const auto& D : DataDirectories)
	// 	{
	// 		Dirs.Add(D.Path);
	// 	}
	// }
	//
	// ItemDataMap.Reset();
	// const int32 Files = UGtItemFactory::LoadItemDataFromDirs(Dirs, ItemDataMap);
	// UE_LOG(LogTemp, Log, TEXT("[ItemManager] Loaded %d files, %d items"), Files, ItemDataMap.Num());
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

AGtItemBase* UGtItemManagerComponent::SpawnItemByIdAt(const FString& ItemId, const FVector& Location, const FRotator& Rotation)
{
	UE_LOG(LogTemp, Warning, TEXT("[ItemTest] SpawnItemByIdAt('%s')"), *ItemId);

	if (FGtItemData* Data = ItemDataMap.Find(ItemId))
	{
		if (AGtItemBase* Item = UGtItemFactory::CreateItem(*Data, GetWorld()))
		{
			Item->SetActorLocation(Location);
			Item->SetActorRotation(Rotation);
			UE_LOG(LogTemp, Warning, TEXT("[ItemTest] Spawned %s at %s"),
				*ItemId, *Location.ToCompactString());
			return Item;
		}
		UE_LOG(LogTemp, Error, TEXT("[ItemTest] CreateItem failed: %s"), *ItemId);
		return nullptr;
	}
	UE_LOG(LogTemp, Error, TEXT("[ItemTest] Id not found: %s (Loaded=%d)"),
		*ItemId, ItemDataMap.Num());
	return nullptr;
}