#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GtEquipmentComponent.generated.h"

class UGtItemManagerComponent;
class AGtWeaponItem;
class AGtItemBase;

// 장비 상태가 변경되었을 때 호출될 델리게이트 (UI, 애니메이션 등에 알림)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentChanged, AGtItemBase*, NewItem);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GIGANTES_API UGtEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGtEquipmentComponent();

	// 지정된 ItemId를 장착
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void EquipWeapon(const FString& ItemId);

	// 현재 장착된 아이템을 해제
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void UnequipWeapon();

	// 장착물의 메인 액션 함수
	void PrimaryAction();

	// 장착물의 서브 액션 함수(조준 및 근접 방어 등)
	void SecondaryAction();
	
	// 재장전을 요청.
	void ReloadAction();

	// 현재 장착된 무기 아이템을 반환합니다.
	UFUNCTION(BlueprintPure, Category = "Equipment")
	AGtWeaponItem* GetCurrentWeapon() const { return CurrentWeapon; }

	// TODO : 테스트 코드로써 삭제 필요
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void EquipTestWeapon();
	
protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Equipment")
	FOnEquipmentChanged OnEquipmentChanged;

private:
	
	// TODO : 테스트용 무기 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	TSubclassOf<AGtWeaponItem> TestWeaponClass;
	
	// 현재 장착된 아이템
	UPROPERTY()
	TObjectPtr<AGtWeaponItem> CurrentWeapon = nullptr;
    
	// TODO: 장비 정보에 장착할 소켓 이름 이전
	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	FName WeaponAttachSocketName = FName("RightHandSocket");

	UPROPERTY()
	TObjectPtr<UGtItemManagerComponent> ItemManagerComponent;
};
