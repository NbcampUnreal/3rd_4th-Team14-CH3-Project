#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gigantes/Items/Runtime/Core/GtItemBase.h"
#include "GtItemManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

class AGtItemBase;
class UTexture2D;
struct FGtItemData;

USTRUCT(BlueprintType)
struct FGtInventoryViewRow
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) int32 Index = INDEX_NONE;
	UPROPERTY(BlueprintReadOnly) FText DisplayName;
	UPROPERTY(BlueprintReadOnly) TObjectPtr<UTexture2D> Icon = nullptr;
	UPROPERTY(BlueprintReadOnly) int32 Count = 1;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GIGANTES_API UGtItemManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGtItemManagerComponent();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Inventory")
	TArray<TObjectPtr<AGtItemBase>> Inventory;

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnInventoryChanged OnInventoryChanged;

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool PickupFromActor(AActor* PickupActor);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool DropItem(int32 Index, const FTransform& Where);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool GiveItemById(const FString& ItemId);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool GiveItemFromData(const FGtItemData& Data);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory")
	TArray<FGtInventoryViewRow> GetInventoryView() const;

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void UseItem(int32 Index);
};