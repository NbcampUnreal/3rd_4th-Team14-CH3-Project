#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "GtCharacterBase.generated.h"

class UGtAttributeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatusTagChanged, FGameplayTag, Tag, bool, bAdded);

UCLASS(Abstract)
class GIGANTES_API AGtCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AGtCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Attribute 헬퍼 함수
	 */
	UFUNCTION(BlueprintCallable, Category="Attributes|Primary")
	float GetAttributePrimary(const FGameplayTag& AttributePrimaryTag) const;

	UFUNCTION(BlueprintCallable, Category="Attributes|Primary")
	void SetAttributePrimary(const FGameplayTag& AttributePrimaryTag, float NewValue);

	UFUNCTION(BlueprintCallable, Category="Attributes|Primary")
	void AddToAttributePrimary(const FGameplayTag& AttributePrimaryTag, float Delta);
	
	/**
	 * Health 전용 헬퍼 함수
	 */
	UFUNCTION(BlueprintCallable, Category="Attributes|Primary")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category="Attributes|Primary")
	void SetHealth(float NewValue);

	UFUNCTION(BlueprintCallable, Category="Attributes|Primary")
	void AddHealth(float Delta);

	/**
	 * Status Tag 헬퍼 함수
	 */
	UFUNCTION(BlueprintCallable, Category="Status")
	bool HasStatusTagExact(const FGameplayTag& StatusTag) const;

	UFUNCTION(BlueprintCallable, Category="Status")
	void AddStatusTag(const FGameplayTag& StatusTag);

	// Dead 태그는 외부에서 제거 불가(Respawn/Reset 등 별도 루틴에서만)
	UFUNCTION(BlueprintCallable, Category="Status")
	void RemoveStatusTag(const FGameplayTag& StatusTag);
	
	
	// TODO : 데미지 관련 로직도 전용 컴포넌트 등으로 위임?
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGtAttributeComponent> AttributeComponent;
	
	UPROPERTY(BlueprintAssignable, Category="Status")
	FOnStatusTagChanged OnStatusTagChanged;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Status")
	FGameplayTagContainer StatusTags;
};
