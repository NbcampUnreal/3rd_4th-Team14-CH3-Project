#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GtAttributeComponent.generated.h"

// Attribute 변경 델리게이트 : 태그, 이전값, 변경 후 값
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributePrimaryChanged, const FGameplayTag&, AttributeTag, float, OldValue, float, NewValue);

USTRUCT(BlueprintType)
struct FAttributePrimaryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxValue = 100.f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GIGANTES_API UGtAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGtAttributeComponent();

	UFUNCTION(BlueprintCallable, Category="Attributes|Primary")
	void SetAttributePrimary(const FGameplayTag& AttributePrimaryTag, const float NewValue);

	UFUNCTION(BlueprintCallable, Category="Attributes|Primary")
	float GetAttributePrimary(const FGameplayTag& AttributePrimaryTag) const;

	UFUNCTION(BlueprintCallable, Category="Attributes|Primary")
	bool GetAttributePrimaryData(const FGameplayTag& AttributePrimaryTag, FAttributePrimaryData& OutAttributePrimaryData) const;
	
protected:
	void BroadcastAttributePrimaryChange(const FGameplayTag& AttributePrimaryTag, const float OldValue, const float NewValue) const;

public:
	UPROPERTY(BlueprintAssignable, Category = "Attributes|Primary")
	FOnAttributePrimaryChanged OnAttributePrimaryChanged;

protected:
	UPROPERTY(EditAnywhere, Category = "Attributes|Primary")
	TMap<FGameplayTag, FAttributePrimaryData> AttributesPrimary;
};
