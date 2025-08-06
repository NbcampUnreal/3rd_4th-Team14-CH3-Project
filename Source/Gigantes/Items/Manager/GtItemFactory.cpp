#include "GtItemFactory.h"

#include "IDetailTreeNode.h"
#include "JsonObjectConverter.h"
#include "Gigantes/Items/Consumables/GtPotion.h"
#include "Gigantes/Items/Granades/GtNormal.h"
#include "Gigantes/Items/Weapons/GtRiffle.h"

/*
 * 맵 / 플래그 정의
 */
TMap<FGameplayTag, TSubclassOf<AGtItemBase>> UGtItemFactory::ItemClassMap;
bool bIsInitialized = false;

void UGtItemFactory::InitItemClassMap()
{
	if (bIsInitialized) return;

	ItemClassMap.Add(FGameplayTag::RequestGameplayTag("Item.Weapon.Riffle"), AGtRiffle::StaticClass());
	ItemClassMap.Add(FGameplayTag::RequestGameplayTag("Item.Weapon.Normal"), AGtNormal::StaticClass());
	ItemClassMap.Add(FGameplayTag::RequestGameplayTag("Item.Weapon.Potion"), AGtPotion::StaticClass());
}

/*
 * JSON 데이터 역직렬화
 */
bool UGtItemFactory::LoadItemData(const FString& FilePath, TMap<FString, FGtItemData>& OutMap)
{
	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *FilePath))
		return false;

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
	TArray<TSharedPtr<FJsonValue>> JsonArray;

	if (!FJsonSerializer::Deserialize(Reader, JsonArray)) return false;

	for (const auto& JsonValue : JsonArray)
	{
		FGtItemData Data;
		if (FJsonObjectConverter::JsonObjectToUStruct(
				JsonValue->AsObject().ToSharedRef(), &Data))
		{
			OutMap.Add(Data.ItemId, Data);
		}
	}

	return true;
}

/*
 * 아이템 생성
 */
AGtItemBase* UGtItemFactory::CreateItem(const FGtItemData& Data, UWorld* World)
{
	TSubclassOf<AGtItemBase>* ClassPtr = ItemClassMap.Find(Data.ItemTag);
	if (!ClassPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemClass] Not Found for Tag: %s"), *Data.ItemTag.ToString());
		return nullptr;
	}
	FActorSpawnParameters Params;
	AGtItemBase* Item = World->SpawnActor<AGtItemBase>(
		*ClassPtr,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		Params
		);

	if (Item)
	{
		Item->InitFromData(Data);
	}
	
	return Item;
}

// AGtItemBase* UGtItemFactory::CreateItem(const FGtItemData& Data, UWorld* World)
// {
// 	if (!World) return nullptr;
//
// 	TSubclassOf<AGtItemBase> ItemClass = nullptr;
//
// 	// SubType에 따라 클래스 분기
// 	if (Data.ItemType == "Weapon")
// 	{
// 		ItemClass = AGtGunItem::StaticClass();
// 	}
// 	else if (Data.ItemType == "Grenade")
// 	{
// 		ItemClass = AGtGrenadeItem::StaticClass();
// 	}
// 	else if (Data.ItemType == "Consumable")
// 	{
// 		ItemClass = AGtHealItem::StaticClass();
// 	}
//
// 	if (!ItemClass) return nullptr;
//
// 	FActorSpawnParameters Params;
// 	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
//
// 	AGtItemBase* NewItem = World->SpawnActor<AGtItemBase>(ItemClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
// 	if (NewItem)
// 	{
// 		NewItem->InitFromData(Data);
// 	}
//
// 	return NewItem;
// }