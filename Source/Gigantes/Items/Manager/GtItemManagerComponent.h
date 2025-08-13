#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gigantes/Items/Base/GtItemBase.h"
#include "Gigantes/Items/Structs/FGtItemData.h"
#include "GtItemManagerComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GIGANTES_API UGtItemManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGtItemManagerComponent();

	UFUNCTION(BlueprintCallable)
	AGtItemBase* SpawnItemByIdAt(const FString& ItemId, const FVector& Location, const FRotator& Rotation);

	UPROPERTY(EditAnywhere, Category="Item|Data")
	TArray<FDirectoryPath> DataDirectories; // 에디터에서 폴더 여러 개 지정

	UPROPERTY(VisibleAnywhere)
	TMap<FString, FGtItemData> ItemDataMap;

	UFUNCTION(BlueprintCallable)
	void LoadAllItemData();
	UFUNCTION(BlueprintCallable)
	void GiveItemToPlayer(const FString& ItemId);
	UFUNCTION(BlueprintCallable)
	void UseItem(int32 Index);

private:
	// TMap<FString, FGtItemData> ItemDataMap;
	TArray<AGtItemBase*> Inventory;
	
};
