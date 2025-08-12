#pragma once

#include "CoreMinimal.h"
#include "Gigantes/Items/Base/GtItemBase.h"
#include "UObject/Object.h"
#include "GtItemFactory.generated.h"

UCLASS()
class GIGANTES_API UGtItemFactory : public UObject
{
	GENERATED_BODY()

public:
	static bool LoadItemData(const FString& FilePath, TMap<FString, FGtItemData>& OutMap);
	// static bool LoadItemDataFile(const FString& FilePath, TMap<FString, FGtItemData>& OutMap);
	static int32 LoadItemDataDir(const FString& DirPath, TMap<FString, FGtItemData>& OutMap, bool bRecursive = true);
	static int32 LoadItemDataFromDirs(const TArray<FString>& Dirs, TMap<FString, FGtItemData>& OutMap);
	
	static AGtItemBase* CreateItem(const FGtItemData& Data, UWorld* World);
	// static void InitItemClassMap();

private:
	static TMap<FGameplayTag, TSubclassOf<AGtItemBase>> TagClassMap;
	static TMap<FName, TSubclassOf<AGtItemBase>> SubtypeMap;

	static void LoadClassMappings();
	
	static bool bIsInitialized;
};
