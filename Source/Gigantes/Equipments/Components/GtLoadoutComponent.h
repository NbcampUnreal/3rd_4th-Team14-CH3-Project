#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gigantes/Items/Structs/FGtItemData.h"
#include "GtLoadoutComponent.generated.h"

class AGtHeroCharacter;
class AGtWeaponItem;
class UGtItemManagerComponent;
class AGtItemBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentItemChanged, AGtItemBase*, NewItem);

USTRUCT(BlueprintType)
struct FGtLoadoutSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment Slot")
	FGameplayTag SlotTag;

	// 이 슬롯에 장착된 아이템의 데이터 (상태 포함)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Equipment Slot")
	FGtItemData EquippedItemData;

	// 로드아웃에 아이템이 등록되어 있는지 확인
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Equipment Slot")
	bool bHasItem = false;
};

/**
 * 플레이어의 퀵슬롯(무기, 수류탄, 소모품)을 통합 관리하는 컴포넌트
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GIGANTES_API UGtLoadoutComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGtLoadoutComponent();

	/**
	 * 아이템 장착/해제 함수로써 아이템 매니저에서 LoadoutComponent를 통해 등록 명령을 내리도록 할 예정
	 */
	UFUNCTION(BlueprintCallable, Category = "Loadout")
	void EquipItemToSlot(const FGtItemData& ItemData, const FGameplayTag& SlotTag);

	UFUNCTION(BlueprintCallable, Category = "Loadout")
	void UnequipItemFromSlot(const FGameplayTag& SlotTag);

	/**
	 * 
	 */
	UFUNCTION(BlueprintCallable, Category = "Loadout")
	void ChangeActiveWeaponSlot(const FGameplayTag& NewActiveSlotTag);

	UFUNCTION(BlueprintCallable, Category = "Loadout")
	void UseItemInSlot(const FGameplayTag& SlotTag);

	// --- 입력 처리 위임 API --- //
	void PrimaryAction();
	void SecondaryAction();
	void ReloadAction();

	// --- 정보 접근 API --- //
	UFUNCTION(BlueprintPure, Category = "Loadout")
	AGtItemBase* GetCurrentEquippedItem() const { return CurrentEquippedItem; }
	
	// TODO : 테스트 코드로써 삭제 필요
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void EquipTestWeapon();
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void DeactivateCurrentWeaponSlot();
	void ActivateNewWeaponSlot(const FGameplayTag& SlotTag);
    
	FGtLoadoutSlot* FindSlotByTag(const FGameplayTag& SlotTag);

public:
	UPROPERTY(BlueprintAssignable, Category = "Loadout")
	FOnEquipmentItemChanged OnEquipmentItemChanged;

private:
	
	// TODO : 테스트용 무기 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Loadout|Test")
	TSubclassOf<AGtWeaponItem> TestWeaponClass;

	UPROPERTY(VisibleAnywhere, Category = "Loadout")
	TArray<FGtLoadoutSlot> LoadoutSlots;
	
	// 현재 관리중인 아이템
	UPROPERTY()
	TObjectPtr<AGtItemBase> CurrentEquippedItem = nullptr;

	UPROPERTY()
	FGameplayTag ActiveWeaponSlotTag;
	
	// TODO: 장비 정보에 장착할 소켓 이름 이전
	UPROPERTY(EditDefaultsOnly, Category = "Loadout")
	FName WeaponAttachSocketName = FName("RightHandSocket");
	
	UPROPERTY()
	TWeakObjectPtr<AGtHeroCharacter> OwnerCharacter;
};
