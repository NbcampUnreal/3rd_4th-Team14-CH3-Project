#pragma once
#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "Gigantes/Items/Runtime/Core/GtItemBase.h"
#include "GtConsumableItem.generated.h"

/** 공통 소모품: UseItem -> ApplyEffect -> (성공 시) 소비 */
UCLASS()
class GIGANTES_API AGtConsumableItem : public AGtItemBase
{
	GENERATED_BODY()
	
public:
	AGtConsumableItem();

	virtual void InitFromData(const FGtItemData& InData) override;

	/** 소비(사용) 시 호출. 보통 캐릭터가 자신의 포션을 사용 */
	UFUNCTION(BlueprintCallable, Category="Consumable")
	virtual bool UseItem(AActor* User);

protected:
	/** 사용 후 파괴할지 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Consumable|Config")
	bool bDestroyOnUse = true;

	/** 사용 쿨타임(초). 0이면 없음 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Consumable|Config")
	float CooldownSeconds = 0.f;

	/** 남은 사용 횟수(스택). 0이면 무제한으로 간주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Consumable|Config")
	int32 Charges = 1;

	/** (옵션) 사용 이펙트/사운드 */
	UPROPERTY(EditDefaultsOnly, Category="Consumable|VFX")
	UNiagaraSystem* UseFX;
	
	UPROPERTY(EditDefaultsOnly, Category="Consumable|SFX")
	USoundBase* UseSFX;

	/** 내부 상태 */
	bool bOnCooldown = false;
	FTimerHandle CooldownHandle;

	/** 실제 효과를 적용하는 훅. 파생에서 구현 */
	virtual bool ApplyEffect(AActor* User);

	/** 사용 이펙트 공통 처리 */
	virtual void PlayUseEffects(AActor* User);

	/** 쿨타임 종료 */
	void EndCooldown();
};