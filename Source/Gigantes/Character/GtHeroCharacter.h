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
	 * TODO: AimOffset 관련 함수들로써 추후 카메라 매니저 등에서 가져올 수 있도록 설정 고려
	 */
	UFUNCTION(BlueprintCallable, Category = "Gameplay|AimOffset")
	float GetAimOffsetYaw() const { return AimOffsetYaw; }
    
	UFUNCTION(BlueprintCallable, Category = "Gameplay|AimOffset")
	float GetAimOffsetPitch() const { return AimOffsetPitch; }

protected:
	virtual void BeginPlay() override;
	// TODO: AimOffset을 위한 Tick으로써 추후 카메라 매니저 등으로 대체
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	void Input_Jump(const FInputActionValue& InputActionValue);

	UFUNCTION()
	void OnLandedCallback(const FHitResult& Hit);
	
	UFUNCTION()
	void OnCharacterStatusTagChanged(const FGameplayTag& StatusTag, bool bAdded);

	
private:

	// TODO: AimOffset을 위한 계산 함수로써 추후 카메라 매니저 등에서 구현 대체
	void CalculateAimOffset();
	
	/**
	 * WallRun 관련 함수
	 */
	void StartWallRunCheck();
	void CheckForWallRun();

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

private:

	/**
	 * AimOffset 관련 변수 TODO: 카메라 매니저 등 구현을 하게 되면 여기서 AimOffset 관리하여 헬퍼 함수로 캐릭터에 전달
	 */
	float AimOffsetYaw = 0.0f;

	float AimOffsetPitch = 0.0f;
	
	// WallRun 조건 확인을 위한 타이머 핸들
	FTimerHandle WallRunCheckTimer;
};
