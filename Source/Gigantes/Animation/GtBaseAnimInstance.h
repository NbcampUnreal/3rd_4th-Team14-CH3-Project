// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "GtBaseAnimInstance.generated.h"

class AGtCharacterBase;
class UCharacterMovementComponent;

struct FGtBaseAnimInstanceProxy  : public FAnimInstanceProxy
{
	FGtBaseAnimInstanceProxy(UAnimInstance* Instance);
	
	// 게임 스레드에서 데이터를 안전하게 수집하는 함수
	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;
	
	// 워커 스레드에서 안전하게 사용할 캐시된 데이터들
	FVector Velocity;
	FVector Acceleration;
	FRotator ActorRotation;
	bool bIsFalling;
	bool bIsMovingOnGround;
	FGameplayTagContainer CachedStatusTags;
	
};

UCLASS()
class GIGANTES_API UGtBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float LocomotionDirection;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bHasVelocity;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bHasAcceleration;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsJumping;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsOnGround;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float FallSpeed; 
	
	// 캐릭터의 현재 상태를 나타내는 게임플레이 태그 컨테이너
	UPROPERTY(BlueprintReadOnly, Category = "State")
	FGameplayTagContainer StatusTags;
	
};
