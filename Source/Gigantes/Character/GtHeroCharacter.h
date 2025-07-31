// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GtHumanBase.h"
#include "GtHeroCharacter.generated.h"

struct FInputActionValue;

class UCameraComponent;
class USpringArmComponent;
class UGtInputConfig;

UCLASS()
class GIGANTES_API AGtHeroCharacter : public AGtHumanBase
{
	GENERATED_BODY()

public:
	AGtHeroCharacter(FObjectInitializer const& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UGtInputConfig> InputConfigDataAsset;


	
};
