// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "GtTurretBase.generated.h"


UCLASS()
class GIGANTES_API AGtTurretBase : public APawn
{
	GENERATED_BODY()

public:
	AGtTurretBase();

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
	int TurretMaxHP;
	int TurretCurrentHP;

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
	
};
