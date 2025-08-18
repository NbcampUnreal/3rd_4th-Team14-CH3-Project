#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UGtInteractionComponent.generated.h"

class IGtInteractable;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GIGANTES_API UGtInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGtInteractionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 상호작용 시도(입력에서 호출)
	UFUNCTION(BlueprintCallable, Category="Interact")
	void TryInteract();

	// 트레이스 세팅
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact")
	float TraceDistance = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact|Trace")
	TArray<TEnumAsByte<ECollisionChannel>> ExtraObjectChannels; // Pawn, PhysicsBody 등

private:
	TWeakObjectPtr<AActor> FocusedActor;

	void UpdateFocus();
	void ClearFocus();
};