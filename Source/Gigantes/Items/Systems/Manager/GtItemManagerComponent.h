#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gigantes/Items/Runtime/Core/GtItemBase.h"
#include "GtItemManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GIGANTES_API UGtItemManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGtItemManagerComponent();
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Inventory")
	TArray<AGtItemBase*> Inventory;

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnInventoryChanged OnInventoryChanged; // BP 바인딩 가능 범위

	UFUNCTION(BlueprintCallable)
	bool GiveItemById(const FString& ItemId); // 반환값으로 성공여부

	UFUNCTION(BlueprintCallable)
	void UseItem(int32 Index);
};