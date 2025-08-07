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
	static AGtItemBase* CreateItem(const FGtItemData& Data, UWorld* World);
	static void InitItemClassMap();

private:
	static TMap<FGameplayTag, TSubclassOf<AGtItemBase>> ItemClassMap;
	static bool bIsInitialized;
};
