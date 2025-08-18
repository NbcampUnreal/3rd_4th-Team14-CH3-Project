#include "GtItemManagerComponent.h"
#include "Gigantes/Items/Systems/Factory/GtItemFactory.h"
#include "Gigantes/Items/Systems/DataSubSystem/GigantesItemDataSubsystem.h"

UGtItemManagerComponent::UGtItemManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UGtItemManagerComponent::GiveItemById(const FString& ItemId)
{
	UWorld* World = GetWorld();
	if (!World) return false;

	FTransform SpawnXform; // 필요 시 소유자 위치/인벤토리 전용 스폰 위치로 교체
	if (AActor* Spawned = UGtItemFactory::SpawnItemById(World, ItemId, SpawnXform))
	{
		if (AGtItemBase* Item = Cast<AGtItemBase>(Spawned))
		{
			Inventory.Add(Item);
			OnInventoryChanged.Broadcast();
			return true;
		}
		// 스폰 타입이 예상과 다르면 정리
		Spawned->Destroy();
	}
	UE_LOG(LogTemp, Warning, TEXT("GiveItemById failed: %s"), *ItemId);
	return false;
}

void UGtItemManagerComponent::UseItem(int32 Index)
{
	if (Inventory.IsValidIndex(Index) && Inventory[Index])
	{
		Inventory[Index]->UseItem();
	}
}