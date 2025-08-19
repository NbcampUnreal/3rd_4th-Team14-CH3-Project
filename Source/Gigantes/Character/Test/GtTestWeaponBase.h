// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gigantes/Equipments/GtEquippable.h"
#include "Gigantes/Items/Runtime/Core/GtItemBase.h"
#include "Gigantes/Player/GtCameraModifierSource.h"
#include "Gigantes/Player/GtPlayerCameraManager.h"
#include "GtTestWeaponBase.generated.h"


class UCurveVector;
// 커브 기반 반동 데이터
USTRUCT(BlueprintType)
struct FGtRecoilData
{
	GENERATED_BODY()
    
	// 시간에 따른 반동 패턴을 정의하는 커브 에셋
	// X축: 발사 횟수(Shot Count), Y축(R): Pitch(수직), Y축(G): Yaw(수평)
	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	TObjectPtr<UCurveVector> RecoilPatternCurve;
    
	// 반동 회복이 시작되기까지의 딜레이 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	float RecoveryDelay = 0.2f;
    
	// 반동 회복 속도
	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	float RecoverySpeed = 8.0f;
    
	// 조준 시 반동 감소 배율
	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	float AimRecoilMultiplier = 0.5f;

	// TEMP 반동 적용 강도
	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	float RecoilIntensity = 1.0f;
};

// 커브 기반 확산 데이터
USTRUCT(BlueprintType)
struct FGtSpreadData
{
	GENERATED_BODY()
    
	// 발사 횟수에 따른 확산도 증가를 정의하는 커브
	// X축: 발사 횟수(Shot Count), Y축: 확산 각도(Degrees)
	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	TObjectPtr<UCurveFloat> SpreadCurve;
    
	// 확산 회복 속도 (초당 감소하는 각도)
	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float SpreadRecoveryRate = 5.0f;
    
	// 최대 확산 각도
	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float MaxSpread = 4.0f;
    
	// 조준 시 확산 감소 배율
	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float AimSpreadMultiplier = 0.3f;
};


// TODO : 이 구조체 정보도 FGtItemData에 추가 예정
UENUM(BlueprintType)
enum class EGtFireMode : uint8
{
	Single      UMETA(DisplayName = "Single"),      // 단발 (권총)
	Automatic   UMETA(DisplayName = "Automatic"),   // 자동 (라이플)
	Burst       UMETA(DisplayName = "Burst"),       // 점사 (선택사항)
};

class USphereComponent;
/**
 * 테스트 전용 무기 베이스 클래스
 * TODO: 정식 무기 시스템 완성 후 제거
 */
UCLASS()
class GIGANTES_API AGtTestWeaponBase : public AGtItemBase, public IGtEquippable, public IGtCameraModifierSource
{
	GENERATED_BODY()

public:
	AGtTestWeaponBase();

	// 테스트용 초기화 함수
	UFUNCTION(BlueprintCallable, Category = "Test")
	void InitializeTestWeapon();
	
	// IGtEquippable 인터페이스 구현
	virtual void OnEquipped_Implementation(AActor* NewOwner) override;
	virtual void OnUnequipped_Implementation() override;
	virtual void ExecutePrimaryActionPressed_Implementation() override;
	virtual void ExecutePrimaryActionReleased_Implementation() override;
	virtual void ExecuteSecondaryActionPressed_Implementation() override;
	virtual void ExecuteSecondaryActionReleased_Implementation() override;
	virtual void ExecuteReloadAction_Implementation() override;

	// IGtCameraModifierSource 인터페이스 구현
	virtual bool GetCameraModifierForTag_Implementation(const FGameplayTag& ActionTag, FGtCameraModifier& OutModifier) const override;

	USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Aim")
	bool IsAiming() const { return bIsAiming; }

	// TODO: ItemData 통합 후 제거
	UFUNCTION(BlueprintPure, Category = "Weapon|Aim")
	float GetAimMovementSpeedMultiplier() const { return AimMovementSpeedMultiplier; }
    
	// TODO: ItemData 통합 후 제거  
	UFUNCTION(BlueprintPure, Category = "Weapon|Aim")
	const FGtCameraModifier& GetAimCameraModifier() const { return AimCameraModifier; }

	// [추가] 애님 노티파이 또는 다른 시스템에서 호출할 공개 함수들
	UFUNCTION(BlueprintCallable)
	void OnNotify_RefillAmmo();
	void EndReload();

	// [추가] 캐릭터가 현재 무기의 재장전 몽타주를 알 수 있도록 Getter 추가
	UAnimMontage* GetCharacterReloadMontage() const { return CharacterReloadMontage; }

protected:
	// 테스트용 간단한 발사 로직
	virtual void TestFire();
	virtual void TestReload();

	// 반동 시스템 (커브 기반)
	void ApplyInstantRecoil();
	void StartRecoilRecovery();
	void ProcessRecoilRecovery();
    
	// 확산 시스템 (커브 기반)
	void ApplyInstantSpread();
	void StartSpreadRecovery();
	void ProcessSpreadRecovery();

	// '2-Trace' 방식의 조준 로직
	bool GetTargetHitResult(FHitResult& OutHitResult) const;

	// VFX 함수
	void PlayMuzzleFlash();
	void StopMuzzleFlash();
	void SpawnTrailEffect(const FVector& Origin, const FVector& Impact);
	
private:
	void StartAiming();
	void StopAiming();

protected:
    //==========================================================================
    // Components
    //==========================================================================
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> Sphere;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> WeaponMesh;

    //==========================================================================
    // Weapon Configuration
    //==========================================================================
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Fire")
    EGtFireMode FireMode = EGtFireMode::Single;
    
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
    FGtRecoilData RecoilData;
    
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Spread")
    FGtSpreadData SpreadData;

    //==========================================================================
    // Aim Configuration
    //==========================================================================
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Aim")
    bool bCanAim = true;
    
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Aim")
    FGtCameraModifier AimCameraModifier;
    
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Aim")
    float AimMovementSpeedMultiplier = 0.5f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Aim")
    bool bUseHoldToAim = true;

    //==========================================================================
    // Animation
    //==========================================================================
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    TSubclassOf<UAnimInstance> ArmedAnimLayer;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Montage")
	TObjectPtr<UAnimMontage> FireRecoilMontage;
	
    //==========================================================================
    // Test Configuration
    //==========================================================================
    UPROPERTY(EditDefaultsOnly, Category = "Test Weapon")
    float TestDamage = 10.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Test Weapon")
    float TestFireRate = 0.1f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Test Weapon")
    int32 TestMaxAmmo = 30;

    //==========================================================================
    // Debug
    //==========================================================================
    UPROPERTY(EditDefaultsOnly, Category = "Test Weapon|Debug")
    bool bShowDebugLine = true;
    
    UPROPERTY(EditDefaultsOnly, Category = "Test Weapon|Debug")
    float DebugLineDuration = 1.0f;

	//==========================================================================
	// Effects
	//==========================================================================
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<UParticleSystem> MuzzleFlashEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float MuzzleFlashScale = 1.0f;
    
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float MuzzleFlashDuration = 0.1f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<UParticleSystem> ImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<UParticleSystem> TrailEffect;
    
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	FName TrailTargetParameterName = TEXT("ShockBeamEnd");

	//==========================================================================
	// Reload Animations
	//==========================================================================
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	UAnimMontage* CharacterReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	UAnimationAsset* WeaponReloadAnimation;

private:
    //==========================================================================
    // Runtime State
    //==========================================================================
    UPROPERTY()
    TObjectPtr<AActor> WeaponOwner;

	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> MuzzleFlashComponent;
    
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Aim", meta = (AllowPrivateAccess = "true"))
    bool bIsAiming = false;

	// [추가] 현재 재장전 중인지 상태를 나타내는 플래그
	UPROPERTY(VisibleAnywhere, Category="Weapon|State")
	bool bIsReloading = false;
    
    // Fire State
    bool bFireInputPressed = false;
    bool bCanFire = true;
    int32 CurrentAmmo;
    
    // Recoil/Spread/ShotCount State
    int32 ConsecutiveShotCount = 0;
    FVector2D AccumulatedRecoil = FVector2D::ZeroVector;
    float CurrentSpread = 0.0f;
	float LastRecoilUpdateTime = 0.0f;
	bool bRecoilRecoveryStarted = false;
	float ShotCountResetDelay = 0.5f;

    // Timer Handles
    FTimerHandle AutoFireTimerHandle;
    FTimerHandle FireTimerHandle;
    FTimerHandle RecoilRecoveryTimer;
    FTimerHandle SpreadRecoveryTimer;
	FTimerHandle ShotCountResetTimer;
	FTimerHandle MuzzleFlashOffTimer;
};
