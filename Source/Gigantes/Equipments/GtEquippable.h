#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GtEquippable.generated.h"

UINTERFACE()
class UGtEquippable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GIGANTES_API IGtEquippable
{
	GENERATED_BODY()

public:
	// 장비(현재는 실제 무기만)가 캐릭터에 장착되었을 때 호출될 이벤트
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equippable")
	void OnEquipped(AActor* NewOwner);

	// 장비(현재는 실제 무기만)가 캐릭터에서 장착 해제되었을 때 호출될 이벤트
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equippable")
	void OnUnequipped();

	// 주 공격 (마우스 좌클릭)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equippable")
	void ExecutePrimaryActionPressed();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equippable")
	void ExecutePrimaryActionReleased();
	
	// 보조 공격 (마우스 우클릭)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equippable")
	void ExecuteSecondaryActionPressed();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ExecuteSecondaryActionReleased();
	
	// 재장전
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equippable")
	void ExecuteReloadAction();
};
