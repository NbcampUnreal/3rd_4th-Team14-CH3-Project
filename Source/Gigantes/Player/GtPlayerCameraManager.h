// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Camera/PlayerCameraManager.h"
#include "GtPlayerCameraManager.generated.h"

class AGtHeroCharacter;

/**
 * 카메라 값을 어떻게 적용할지 정의
 */
UENUM(BlueprintType)
enum class EGtCameraValueOperation : uint8
{
	// 값을 더함 (기본값 + 모디파이어 값) 
	Additive,
	// 값을 덮어씀 (모디파이어 값으로 대체)
	Override
};

/**
 * 캐릭터의 '베이스' 상태(서기, 웅크리기 등)에 대한 카메라 옵션
 */
USTRUCT(BlueprintType)
struct FGtCameraOption
{
	GENERATED_BODY()

	// Z축 추가 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Option")
	float CameraOffsetZ = 0.0f;

	// 시야각 (Field of View)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Option")
	float FOV = 90.0f;

	// 스프링암 길이 (카메라와의 거리)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Option")
	float SpringArmLength = 300.0f;

	// 이 액션으로 전환될 때의 카메라 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Option")
	float TransitionSpeed = 10.0f;
};

/**
 *  캐릭터의 '액션' 상태(조준, 피격 등)를 위한 카메라 모디파이어(TODO: 무기에서 FOV(에임 상태)등의 정보를 가지고 있는게 어울려 보임)
 */
USTRUCT(BlueprintType)
struct FGtCameraModifier
{
	GENERATED_BODY()

	// 모디파이어 적용 우선순위(높을수록 나중에 적용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Modifier", meta = (DisplayPriority = 1))
	int32 Priority = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Modifier")
	EGtCameraValueOperation CameraOffsetZ_Op = EGtCameraValueOperation::Additive;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Modifier")
	float CameraOffsetZ = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Modifier")
	EGtCameraValueOperation FOV_Op = EGtCameraValueOperation::Override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Modifier")
	float FOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Modifier")
	EGtCameraValueOperation SpringArmLength_Op = EGtCameraValueOperation::Additive;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Modifier")
	float SpringArmLength = 0.0f;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Modifier")
	float TransitionSpeed = 10.0f;
};

struct FModifierPair
{
	FGameplayTag Tag;
	FGtCameraModifier Modifier;
};

UCLASS()
class GIGANTES_API AGtPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
public:
	AGtPlayerCameraManager();

	UFUNCTION(BlueprintPure, Category = "Camera")
	float GetAimOffsetYaw() const { return CurrentAimOffsetYaw; }
    
	UFUNCTION(BlueprintPure, Category = "Camera")
	float GetAimOffsetPitch() const { return CurrentAimOffsetPitch; }
	
protected:
	virtual void SetViewTarget(AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams()) override;
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// 캐릭터의 상태 변경 델리게이트에 바인딩될 핸들러 함수
	UFUNCTION()
	void OnCharacterStatusTagChanged(const FGameplayTag& StatusTag, bool bAdded);

	// 카메라의 최종 위치 및 옵션을 계산하고 적용하는 함수
	void ApplyCameraBehavior(FTViewTarget& OutVT, float DeltaTime);
	void UpdateAimOffset(const FTViewTarget& VT, float DeltaTime);

	// 캐릭터의 현재 상태를 평가하여 카메라의 목표값을 갱신하는 함수 
	void UpdateCameraTargets();

	// 베이스 상태 결정 헬퍼 함수
	FGameplayTag DetermineBaseState(const AGtHeroCharacter* HeroCharacter) const;
    
	// 모디파이어 적용 헬퍼 함수
	void ApplyModifiers(const FGameplayTagContainer& StatusTags);
	
	AGtHeroCharacter* GetHeroCharacter() const;

protected:
	// 베이스 상태 우선순위 (높은 인덱스가 높은 우선순위)
	UPROPERTY(EditDefaultsOnly, Category = "Camera Options|Priority")
	TArray<FGameplayTag> BaseStatePriority;
	
	// 상태 태그를 키로, 카메라 베이스 옵션을 값으로 갖는 TMap 
	UPROPERTY(EditDefaultsOnly, Category = "Camera Options|Base")
	TMap<FGameplayTag, FGtCameraOption> BaseCameraOptionsMap;

	// TODO: 여기서 데이터 관리보다 무기에서 카메라 정보 가지고 있는게 좋아보임 
	UPROPERTY(EditDefaultsOnly, Category = "Camera Options|Modifiers")
	TMap<FGameplayTag, FGtCameraModifier> ActionModifierMap;

	// 어떤 상태에도 해당하지 않을 때(서 있을 때 등) 사용할 기본 옵션
	UPROPERTY(EditDefaultsOnly, Category = "Camera Options|Base")
	FGtCameraOption DefaultCameraOptions;

private:
	
	float CurrentCameraOffset = 0.0f;
	float TargetCameraOffset = 0.0f;

	float CurrentFOV = 90.0f;
	float TargetFOV = 90.0f;

	float CurrentSpringArmLength = 300.0f;
	float TargetSpringArmLength = 300.0f;

	float CurrentTransitionSpeed = 10.0f;
	
	float CurrentAimOffsetYaw = 0.0f;
	float CurrentAimOffsetPitch = 0.0f;

	FGameplayTag CurrentBaseStateTag;
	FGameplayTagContainer PreviousStatusTags;
	
	// 캡슐 높이 캐싱 (초기화용)
	float CachedStandingCapsuleHalfHeight = 0.0f;

	// ViewTarget 캐싱(TODO: 추후 로못 캐릭터 구현 시 이렇게 Hero전용으로 하면 안됨)
	UPROPERTY()
	TWeakObjectPtr<AGtHeroCharacter> CachedHeroCharacter;
};
