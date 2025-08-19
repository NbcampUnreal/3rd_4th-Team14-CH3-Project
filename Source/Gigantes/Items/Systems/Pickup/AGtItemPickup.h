#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gigantes/Items/Structs/FGtItemData.h"
#include "Gigantes/Items/Systems/Interaction/GtInteractable.h"
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

public:
	// 어떤 아이템을 줄지(월드 배치 시 세팅)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup")
	FGtItemData ItemData;

	// ===== IGtInteractable 구현 =====
	virtual void BeginFocus_Implementation(AActor* Interactor) override;
	virtual void EndFocus_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
};