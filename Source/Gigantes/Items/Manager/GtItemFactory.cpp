#include "GtItemFactory.h"

#include "JsonObjectConverter.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Dom/JsonValue.h"
#include "Gigantes/Items/Data/ItemClassMapping.h"

/*
 * 맵 / 플래그 정의
 */
TMap<FGameplayTag, TSubclassOf<AGtItemBase>> UGtItemFactory::TagClassMap;
TMap<FName, TSubclassOf<AGtItemBase>> UGtItemFactory::SubtypeMap;
bool UGtItemFactory::bIsInitialized = false;

/*
 * JSON 데이터 역직렬화
 */
bool UGtItemFactory::LoadItemData(const FString& FilePath, TMap<FString, FGtItemData>& OutMap)
{
	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("[ItemFactory] Failed to load JSON file: %s"), *FilePath);
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("[ItemFactory] JSON file loaded successfully."));
	
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	auto Reader = TJsonReaderFactory<>::Create(JsonStr);

	if (!FJsonSerializer::Deserialize(Reader, JsonArray) || JsonArray.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[ItemFactory] Failed to parse JSON or JSON is empty."));
		return false;
	}

	int32 Added = 0;
	for (const auto& JsonValue : JsonArray)
	{
		const TSharedPtr<FJsonObject> Obj = JsonValue->AsObject();
		if (!Obj.IsValid())
			continue;
		
		FGtItemData Data;
			
		if (FJsonObjectConverter::JsonObjectToUStruct(
				JsonValue->AsObject().ToSharedRef(), &Data))
		{
			// ★ 수동 후처리
			FString TagStr, ClassStr;
			if (Obj->TryGetStringField(TEXT("ItemTag"), TagStr) && !TagStr.IsEmpty())
			{
				Data.ItemTag = FGameplayTag::RequestGameplayTag(FName(*TagStr));
			}
			if (Obj->TryGetStringField(TEXT("ClassPath"), ClassStr) && !ClassStr.IsEmpty())
			{
				Data.ClassPath = FSoftClassPath(ClassStr);
			}
			
			OutMap.Add(Data.ItemId, Data);
			++Added;

			UE_LOG(LogTemp, Log, TEXT("[ItemFactory] Parsed: Id=%s, Name=%s, Tag=%s, Class=%s, Dmg=%d"),
			*Data.ItemId, *Data.ItemName,
			*Data.ItemTag.ToString(),
			*Data.ClassPath.ToString(),
			Data.Damage);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("[ItemFactory] %d %s"), Added, *FilePath);
	return Added > 0;
}

int32 UGtItemFactory::LoadItemDataDir(const FString& DirPath, TMap<FString, FGtItemData>& OutMap, bool bRecursive)
{
	TArray<FString> Files;
	IFileManager::Get().FindFilesRecursive(
		Files, *DirPath, TEXT("*.json"), true /*files*/, false /*dirs*/, bRecursive);

	int32 Total=0;
	for (const FString& F : Files)
	{
		if (LoadItemData(F, OutMap)) { ++Total; }
	}
	UE_LOG(LogTemp, Log, TEXT("[ItemFactory] Loaded %d json files from %s"), Total, *DirPath);
	return Total;
}

int32 UGtItemFactory::LoadItemDataFromDirs(const TArray<FString>& Dirs, TMap<FString, FGtItemData>& OutMap)
{
	int32 Sum=0;
	for (const FString& D : Dirs) { Sum += LoadItemDataDir(D, OutMap, true); }
	return Sum;
}


void UGtItemFactory::LoadClassMappings()
{
	if (bIsInitialized) return;

	// 프로젝트의 UDataAsset 경로를 지정합니다.
	// 프로젝트 설정(Config) 파일에서 이 경로를 가져오는 것이 더 좋은 방법입니다.
	FSoftObjectPath MappingAssetPath(TEXT("/Game/Items/DA/ItemClassMapping.ItemClassMapping"));
	UItemClassMapping* MappingAsset = Cast<UItemClassMapping>(MappingAssetPath.TryLoad());

	if (!MappingAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("[ItemFactory] Failed to load UItemClassMapping DataAsset. Path: %s"), *MappingAssetPath.ToString());
		return;
	}

	TagClassMap.Reset();
	for (const FItemClassMappingRow& Mapping : MappingAsset->Mappings)
	{
		if (Mapping.ItemTag.IsValid() && Mapping.ItemClass.IsValid())
		{
			// TagClassMap.Add(Mapping.ItemTag, Mapping.ItemClass.LoadSynchronous());
			UClass* Cls = Mapping.ItemClass.LoadSynchronous();
			TagClassMap.Add(Mapping.ItemTag, Cls);
			UE_LOG(LogTemp, Log, TEXT("[ItemFactory] Map %s -> %s"),
				*Mapping.ItemTag.ToString(),
				Cls ? *Cls->GetPathName() : TEXT("NULL"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[ItemFactory] Skip invalid mapping (Tag or Class invalid)"));
		}
	}

	bIsInitialized = true;
	UE_LOG(LogTemp, Log, TEXT("[ItemFactory] Loaded %d class mappings."), TagClassMap.Num());
}

/*
 * 아이템 생성
 */
AGtItemBase* UGtItemFactory::CreateItem(const FGtItemData& Data, UWorld* World)
{
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[ItemFactory] World is null"));
		return nullptr;
	}
	
	LoadClassMappings(); // 매핑 정보를 로드하여 맵을 초기화

	UClass* ChosenClass = nullptr;

	// 1) JSON 데이터의 ClassPath를 가장 우선적으로 사용
	if (Data.ClassPath.IsValid())
	{
		ChosenClass = Data.ClassPath.TryLoadClass<AGtItemBase>();
		UE_LOG(LogTemp, Log, TEXT("[ItemFactory] Try ClassPath: %s -> %s"),
			*Data.ClassPath.ToString(),
			ChosenClass ? *ChosenClass->GetName() : TEXT("NULL"));
	}

	// 2) ClassPath가 없으면 ItemTag를 사용하여 매핑된 클래스를 찾음
	if (!ChosenClass && Data.ItemTag.IsValid())
	{
		if (const auto Found = TagClassMap.Find(Data.ItemTag))
		{
			ChosenClass = Found->Get();
			UE_LOG(LogTemp, Log, TEXT("[ItemFactory] From Tag %s -> %s"),
				*Data.ItemTag.ToString(),
				ChosenClass ? *ChosenClass->GetName() : TEXT("NULL"));
		}
	}

	// 서브타입 매핑 로직은 제거하여 단순화
	if (!ChosenClass)
	{
		FString Keys;
		for (const auto& It : TagClassMap)
		{
			Keys += It.Key.ToString() + TEXT(" ");
		}

		UE_LOG(LogTemp, Error, TEXT("[ItemFactory] Class resolve failed for %s (Tag=%s, ClassPath=%s). Keys={ %s }"),
			*Data.ItemId,
			*Data.ItemTag.ToString(),
			*Data.ClassPath.ToString(),
			*Keys);
		
		return nullptr;
	}

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGtItemBase* Item = World->SpawnActor<AGtItemBase>(
		ChosenClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		P);
	
	if (!Item)
	{
		UE_LOG(LogTemp, Error, TEXT("[ItemFactory] Spawn failed for class %s"),
			*ChosenClass->GetPathName());
		return nullptr;
	}

	Item->InitFromData(Data);

	const FGtItemData& D = Item->GetItemData();
	UE_LOG(LogTemp, Log, TEXT("[ItemFactory] Spawned: %s (%s) Dmg=%d Ammo=%d/%d Rate=%.3f Tag=%s Class=%s"),
		*D.ItemName,
		*D.ItemId,
		 D.Damage,
		 D.AmmoInMagazine,
		 D.MaxAmmo,
		 D.FireRate,
		*D.ItemTag.ToString(),
		*D.ClassPath.ToString());

	return Item;
	
}