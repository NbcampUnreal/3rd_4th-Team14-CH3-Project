// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gigantes/Equipments/GtEquippable.h"
#include "Gigantes/Items/Weapons/GtWeaponItem.h"
#include "Gigantes/Player/GtCameraModifierSource.h"
#include "Gigantes/Player/GtPlayerCameraManager.h"
#include "GtTestWeaponBase.generated.h"

// 이 구조체 정보도 FGtItemData에 추가 예정
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
class GIGANTES_API AGtTestWeaponBase : public AGtWeaponItem, public IGtEquippable, public IGtCameraModifierSource
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


protected:
	// 테스트용 간단한 발사 로직
	virtual void TestFire();
	virtual void TestReload();

	bool GetCrosshairHitResult(FHitResult& OutHitResult) const;

private:
	void StartAiming();
	void StopAiming();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sphere")
	TObjectPtr<USphereComponent> Sphere;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;
	
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Aim")
	bool bIsAiming = false;

	// TODO: FGtItemData에 추가 후 ItemData.bCanAim으로 이전 예정
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Aim")
	bool bCanAim = true;
    
	// TODO: FGtItemData에 추가 후 ItemData.AimCameraModifier로 이전 예정
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Aim")
	FGtCameraModifier AimCameraModifier;
    
	// TODO: FGtItemData에 추가 후 ItemData.AimMovementSpeedMultiplier로 이전 예정
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Aim")
	float AimMovementSpeedMultiplier = 0.5f;

	// TODO : 추후 WeakObjectPtr로 변경 or 삭제후 Owner(Outer) 사용 고려
	UPROPERTY()
	TObjectPtr<AActor> WeaponOwner;

	// TODO : 조준 모드 설정
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Aim")
	bool bUseHoldToAim = true;  // true: 홀딩, false: 토글

	// TODO : 발사 모드
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Fire")
	EGtFireMode FireMode = EGtFireMode::Single;
    
	// TODO : 자동 발사용 타이머
	FTimerHandle AutoFireTimerHandle;
    
	// TODO: 발사 입력 상태
	bool bFireInputPressed = false;
	
	// 테스트용 기본값
	UPROPERTY(EditDefaultsOnly, Category = "Test Weapon")
	float TestDamage = 10.0f;
    
	UPROPERTY(EditDefaultsOnly, Category = "Test Weapon")
	float TestFireRate = 0.1f;
    
	UPROPERTY(EditDefaultsOnly, Category = "Test Weapon")
	int32 TestMaxAmmo = 30;

	// 디버그 표시 옵션
	UPROPERTY(EditDefaultsOnly, Category = "Test Weapon|Debug")
	bool bShowDebugLine = true;
    
	UPROPERTY(EditDefaultsOnly, Category = "Test Weapon|Debug")
	float DebugLineDuration = 1.0f;
	
	int32 CurrentAmmo;
    
	FTimerHandle FireTimerHandle;
	bool bCanFire = true;

	// TODO: 테스트용 애니메이션 레이어 FGtItemData에 추가
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> ArmedAnimLayer;
};
