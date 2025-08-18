#pragma once
#include "CoreMinimal.h"
#include "Gigantes/Items/Runtime/Consumables/GtConsumableItem.h"
#include "GtPotion.generated.h"

/**
 * 포션: HealAmount 만큼 즉시 회복(기본).
 * - C++ 기본: UGtHealthComponent가 붙어 있으면 Find해서 Heal 호출
 * - 없으면 BP 이벤트 훅으로 처리(디자이너 구현)
 */
UCLASS()
class GIGANTES_API AGtPotion : public AGtConsumableItem
{
	GENERATED_BODY()
public:
	AGtPotion();

	virtual void InitFromData(const FGtItemData& InData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Potion|Config")
	int32 HealAmount = 0;

	/** 회복 적용 로직(성공 시 true) */
	virtual bool ApplyEffect(AActor* User) override;

	/** BP에서 회복을 커스텀 처리하고 싶을 때(없으면 false 반환) */
	UFUNCTION(BlueprintImplementableEvent, Category="Potion")
	bool BP_ApplyHeal(AActor* User, int32 InHealAmount);
};