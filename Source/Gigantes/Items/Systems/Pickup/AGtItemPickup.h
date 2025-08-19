#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/Structs/FGtItemData.h"
#include "Items/Systems/Interaction/GtInteractable.h"
#include "AGtItemPickup.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;

UCLASS()
class GIGANTES_API AGtItemPickup : public AActor, public IGtInteractable
{
	GENERATED_BODY()

public:
	AGtItemPickup();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pickup")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pickup")
	UWidgetComponent* PromptWidget;

	virtual void BeginPlay() override;

public:
	/** JSON의 ItemId. 최소 구성은 이 값만 있으면 됨 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup|Data")
	FString ItemId;

	/** (옵션) per-instance 덮어쓰기용 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup|Data", meta=(EditCondition="bUseOverrideData", EditConditionHides))
	FGtItemData ItemDataOverride;

	/** (옵션) 위 오버라이드 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup|Data", meta=(InlineEditConditionToggle))
	bool bUseOverrideData = false;

	/** 인터랙트 가능 거리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pickup|Config")
	float InteractDistance = 250.f;

	// IGtInteractable
	virtual void BeginFocus_Implementation(AActor* Interactor) override;
	virtual void EndFocus_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

private:
	bool GiveToInventory(AActor* Interactor);
};