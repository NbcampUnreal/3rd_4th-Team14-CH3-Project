// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GtHeroMovementComponent.generated.h"

class AGtHeroCharacter;

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMM_None UMETA(DisplayName = "None"),
	CMM_WallRun UMETA(DisplayName = "WallRun"),
	
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GIGANTES_API UGtHeroMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UGtHeroMovementComponent();

	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
	
	void TryEnterWallRun(bool& bOutWallRunIsPossible, bool& bOutIsRightWall);
	void EndWallRun(const FHitResult* FloorHitOption = nullptr);
	
	UFUNCTION(BlueprintPure, Category = "Movement|WallRun")
	FVector GetWallRunNormal() const { return WallRunNormal; }

protected:
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	void PhysWallRun(float DeltaTime, int32 Iterations);

public:

	/**
	 * WallRun 관련 속성들
	 */
	
	// 월런 시작을 위한 최소 수평 속도
	UPROPERTY(EditDefaultsOnly, Category = "Movement|WallRun")
	float WallRunMinSpeed = 500.f; 
 
	// 월런을 시작하기 위한 최소 공중 높이 (바닥에서 바로 타는 것 방지)
	UPROPERTY(EditDefaultsOnly, Category = "Movement|WallRun")
	float WallRunMinHeight = 50.f; 
 
	// 월런 중 적용될 중력 배율
	UPROPERTY(EditDefaultsOnly, Category = "Movement|WallRun")
	float WallRunGravityScale = 0.2f; 
 
	// 월 점프 시 벽에서 옆으로 밀어내는 힘
	UPROPERTY(EditDefaultsOnly, Category = "Movement|WallRun")
	float WallRunJumpOffForce = 500.f; 

	// 월런 중 최대 속도
	UPROPERTY(EditDefaultsOnly, Category = "Movement|WallRun")
	float WallRunMaxSpeed = 800.f;

	// 월런 중 가속도
	UPROPERTY(EditDefaultsOnly, Category = "Movement|WallRun")
	float WallRunAcceleration = 2000.f;
	
	// 월런 감속(마찰) 값 - 초당 몇 cm/s 줄일지
	UPROPERTY(EditDefaultsOnly, Category = "Movement|WallRun")
	float WallRunBrakingDeceleration = 800.f;

	// 캐릭터 캡슐과 벽 사이에 유지할 거리(여유 공간)
	UPROPERTY(EditDefaultsOnly, Category = "Movement|WallRun")
	float WallRunOffsetDistance = 5.f;

	// 월런 중 캐릭터 회전 속도
	UPROPERTY(EditDefaultsOnly, Category = "Movement|WallRun")
	float WallRunRotationSpeed = 10.0f;

protected:
	UPROPERTY()
	TObjectPtr<AGtHeroCharacter> HeroCharacterOwner   =  nullptr;
	
private:
	FVector WallRunNormal = FVector::ZeroVector;
};
