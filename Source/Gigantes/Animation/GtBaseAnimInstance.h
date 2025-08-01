// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimInstance.h"
#include "GtBaseAnimInstance.generated.h"

class AGtCharacterBase;
class UCharacterMovementComponent;

/**
 * 
 */
UCLASS()
class GIGANTES_API UGtBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float LocomotionDirection;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsInAir;

	// 캐릭터의 현재 상태를 나타내는 게임플레이 태그 컨테이너
	UPROPERTY(BlueprintReadOnly, Category = "State")
	FGameplayTagContainer StatusTags;

private:
	TWeakObjectPtr<AGtCharacterBase> OwningCharacter;
	TWeakObjectPtr<UCharacterMovementComponent> MovementComponent;
};
