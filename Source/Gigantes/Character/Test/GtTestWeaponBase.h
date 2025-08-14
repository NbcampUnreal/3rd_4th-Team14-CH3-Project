// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gigantes/Equipments/GtEquippable.h"
#include "Gigantes/Items/Weapons/GtWeaponItem.h"
#include "GtTestWeaponBase.generated.h"

/**
 * 테스트 전용 무기 베이스 클래스
 * TODO: 정식 무기 시스템 완성 후 제거
 */
UCLASS()
class GIGANTES_API AGtTestWeaponBase : public AGtWeaponItem, public IGtEquippable
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
	virtual void ExecutePrimaryAction_Implementation() override;
	virtual void ExecuteSecondaryAction_Implementation() override;
	virtual void ExecuteReloadAction_Implementation() override;

protected:
	// 테스트용 간단한 발사 로직
	virtual void TestFire();
	virtual void TestReload();

	bool GetCrosshairHitResult(FHitResult& OutHitResult) const;

	// TODO : 추후 WeakObjectPtr로 변경 or 삭제후 Owner(Outer) 사용 고려
	UPROPERTY()
	TObjectPtr<AActor> WeaponOwner;
    
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

	// TODO: 테스트용 애니메이션 레이어
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> ArmedAnimLayer;
};
