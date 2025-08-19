#include "GtItemFactory.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/SoftObjectPtr.h"

#include "Gigantes/Items/Structs/FGtItemData.h"
#include "Gigantes/Items/Systems/DataSubSystem/GigantesItemDataSubsystem.h"

AActor* UGtItemFactory::SpawnItemById(UWorld* World, const FString& ItemId, const FTransform& SpawnTransform)
{
	if (!World) return nullptr;

	if (UGigantesItemDataSubsystem* Subsys = World->GetGameInstance()->GetSubsystem<UGigantesItemDataSubsystem>())
	{
		if (const FGtItemData* ItemData = Subsys->FindItemData(ItemId))
		{
			if (UClass* ItemClass = ItemData->ItemClass.LoadSynchronous())
			{
				return SpawnItem(World, *ItemData, SpawnTransform);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("UGtItemFactory::SpawnItemById - ItemId %s not found"), *ItemId);
	return nullptr;
}

AActor* UGtItemFactory::SpawnItem(UWorld* World, const FGtItemData& ItemData, const FTransform& SpawnTransform)
{
	if (!World) return nullptr;

	UClass* ItemClass = ItemData.ItemClass.LoadSynchronous(); // ClassPath 우선
	if (!ItemClass)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnItem: Failed to load class for %s"), *ItemData.ItemId);
		return nullptr;
	}

	AActor* Spawned = World->SpawnActor<AActor>(ItemClass, SpawnTransform);
	if (AGtItemBase* Item = Cast<AGtItemBase>(Spawned))
	{
		// 스폰 -> 데이터 바인딩
		Item->InitFromData(ItemData);
	}
	return Spawned;
}