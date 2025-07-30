#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "Gigantes/Gameplay/Damage/GtDamageable.h"
#include "GtCharacterBase.generated.h"

struct FGtDamageResult;

class UGtAttributeComponent;
class UGtDamageReceiverComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatusTagChanged, FGameplayTag, Tag, bool, bAdded);
DECLARE_DELEGATE_TwoParams(FAttributeChangedHandler, float /*OldValue*/, float /*NewValue*/);

UCLASS(Abstract)
class GIGANTES_API AGtCharacterBase : public ACharacter, public IGtDamageable
{
	GENERATED_BODY()

public:
	AGtCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool ApplyDamage_Implementation(const FGtDamageInfo& DamageInfo, FGtDamageResult& OutDamageResult) override;
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

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION()
	virtual void OnAttributePrimaryChanged(const FGameplayTag& AttributePrimaryTag, float OldValue, float NewValue);
	
	void HandleHealthChanged(float OldValue, float NewValue);
	
	virtual void Die();

	UFUNCTION()
	void HandleDamageResult(const FGtDamageResult& DamageResult);
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGtAttributeComponent> AttributeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGtDamageReceiverComponent> DamageReceiverComponent;
	
	UPROPERTY(BlueprintAssignable, Category="Status")
	FOnStatusTagChanged OnStatusTagChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Status")
	FGameplayTagContainer StatusTags;

	TMap<FGameplayTag, FAttributeChangedHandler> AttributeChangedHandlers;
};
