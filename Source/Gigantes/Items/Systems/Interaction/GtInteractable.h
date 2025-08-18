#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GtInteractable.generated.h"

UINTERFACE(BlueprintType)
class GIGANTES_API UGtInteractable : public UInterface
{
	GENERATED_BODY()
};

class GIGANTES_API IGtInteractable
{
	GENERATED_BODY()

public:
	// 포커스 시작/종료 (UI 프롬프트, 하이라이트 등)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interactable")
	void BeginFocus(AActor* Interactor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interactable")
	void EndFocus(AActor* Interactor);

	// 상호작용 가능 여부(거리, 조건 등)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interactable")
	bool CanInteract(AActor* Interactor) const;

	// 실제 상호작용 (아이템 줍기 등)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interactable")
	void Interact(AActor* Interactor);
};