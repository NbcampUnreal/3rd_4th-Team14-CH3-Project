#include "GtItemFactory.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/SoftObjectPtr.h"

#include "Gigantes/Items/Structs/FGtItemData.h"
#include "Gigantes/Items/Systems/DataSubSystem/GigantesItemDataSubsystem.h"
#include "Kismet/GameplayStatics.h"

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

AGtItemBase* UGtItemFactory::SpawnItem(UWorld* World, const FGtItemData& ItemData, const FTransform& Xf)
{
	if (!World) return nullptr;
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return nullptr;

	auto* Sub = GI->GetSubsystem<UGigantesItemDataSubsystem>();
	if (!Sub || !Sub->IsPreloadFinished()) return nullptr;

	UClass* Cls = Sub->GetHardClassByTag(ItemData.ItemTag);
	if (!IsValid(Cls))
	{
		Cls = ItemData.ItemClass.IsValid() ? ItemData.ItemClass.Get() : ItemData.ItemClass.LoadSynchronous();
	}
	if (!IsValid(Cls)) return nullptr;

	if (!Cls->IsChildOf(AGtItemBase::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("[ItemFactory] Class %s is not AGtItemBase."), *GetNameSafe(Cls));
		return nullptr;
	}

	AGtItemBase* Item = World->SpawnActorDeferred<AGtItemBase>(
		Cls, Xf, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	if (!Item) return nullptr;

	Item->InitFromData(ItemData);
	UGameplayStatics::FinishSpawningActor(Item, Xf);
	return Item;
}