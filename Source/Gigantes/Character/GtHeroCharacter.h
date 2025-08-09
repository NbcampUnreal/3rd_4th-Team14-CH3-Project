// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GtHumanBase.h"
#include "GtHeroCharacter.generated.h"

struct FInputActionValue;

class UGtHeroMovementComponent;
class UCameraComponent;
class USpringArmComponent;
class UGtInputConfig;

UCLASS()
class GIGANTES_API AGtHeroCharacter : public AGtHumanBase
{
	GENERATED_BODY()

public:
	AGtHeroCharacter(const FObjectInitializer & ObjectInitializer = FObjectInitializer::Get());

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;

	/**
	 * Crouch 관련 함수들
	 */
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual bool CanCrouch() const override;

	UFUNCTION(BlueprintCallable, Category = "Character")
	virtual void Sprint();
    
	UFUNCTION(BlueprintCallable, Category = "Character")
	virtual void UnSprint();
    
	virtual bool CanSprint() const;
	
	void OnStartSprint();
	void OnEndSprint();
	
	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	UGtHeroMovementComponent* GetHeroMovementComponent() const { return HeroMovementComponent; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	void Input_Jump(const FInputActionValue& InputActionValue);
	void Input_Crouch(const FInputActionValue& InputActionValue);
	void Input_SprintStart(const FInputActionValue& InputActionValue);
	void Input_SprintStop(const FInputActionValue& InputActionValue);


	UFUNCTION()
	void OnLandedCallback(const FHitResult& Hit);
	
	UFUNCTION()
	void OnCharacterStatusTagChanged(const FGameplayTag& StatusTag, bool bAdded);

	bool ShouldStartSlide() const;
	void StartSlide();
	
private:

	/**
	 * MovementComponent 델리게이트 핸들러
	 */
	void HandleCapsuleSizeChanged(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

public:
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	uint8 bIsSprinting : 1;
	
protected:
	UPROPERTY()
	TObjectPtr<UGtHeroMovementComponent> HeroMovementComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UGtInputConfig> InputConfigDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jump")
	int32 MaxJumpCount = 2;
	
	int32 JumpCount = 0;

};
