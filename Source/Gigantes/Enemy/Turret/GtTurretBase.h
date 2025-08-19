// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "Gigantes/Gameplay/Damage/GtDamageable.h"
#include "GtTurretBase.generated.h"


UCLASS()
class GIGANTES_API AGtTurretBase : public APawn, public IGtDamageable
{
	GENERATED_BODY()

public:
	AGtTurretBase();

	// ApplyDamage 인터페이스 함수 오버라이드 
	virtual bool ApplyDamage_Implementation(const FGtDamageInfo& DamageInfo, FGtDamageResult& OutDamageResult) override;

	UFUNCTION(BlueprintPure, Category = "Turret")
	float GetCurrentHP() const { return TurretCurrentHP; }
    
	UFUNCTION(BlueprintPure, Category = "Turret")
	float GetMaxHP() const { return TurretMaxHP; }
	
protected:
	virtual void BeginPlay() override;
	
	//mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	class USkeletalMeshComponent* TurretMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	USphereComponent* Collision;

	//boolean
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret")
	bool bIsFindEnermy;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret")
	bool bIsReadyToAttack;
	
	//timer
	FTimerHandle FindEnermyHandle;
	FTimerHandle AttackReadyHandle;
	FTimerHandle ReloadHandle;
	UFUNCTION()
	void EnermySearchTimerReset();
	UFUNCTION()
	void AttackTimerReset();
	UFUNCTION()
	void ReloadTimerReset();
	
	//TargetActor
	AActor* FindEnemyActor;
	
	//Action
	void EnermySearch();
	UFUNCTION()
	void LookAt();
	void Attack();
	void Reload();
	
	//status
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret")
	float TurretDamage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret")
	float TurretReloadTime;
	float TurretMaxHP;
	float TurretCurrentHP;

	// 죽음 상태 체크
	bool bIsDead = false;
	
	//setter
	void SetHP(int value);

	
public:
	//event
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
					   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
					   bool bFromSweep, const FHitResult& SweepResult);
    
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
					 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	//getter
	int GetHP();

	void Die();
};
