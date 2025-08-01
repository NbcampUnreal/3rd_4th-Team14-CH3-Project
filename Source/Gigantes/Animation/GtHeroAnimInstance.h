// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GtBaseAnimInstance.h"
#include "GtHeroAnimInstance.generated.h"

class AGtHeroCharacter;
/**
 * 
 */
UCLASS()
class GIGANTES_API UGtHeroAnimInstance : public UGtBaseAnimInstance
{
	GENERATED_BODY()
protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "AimOffset")
	float AimOffsetPitch;
	
	UPROPERTY(BlueprintReadOnly, Category = "AimOffset")
	float AimOffsetYaw;
private:
	TWeakObjectPtr<AGtHeroCharacter> OwningHeroCharacter;
};
