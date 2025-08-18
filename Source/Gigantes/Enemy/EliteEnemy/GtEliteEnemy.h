// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Containers/Array.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "GtEliteEnemy.generated.h"

UENUM(BlueprintType)
enum class EBossState : uint8
{
	Spawn			UMETA(DisplayName = "Spawn"),
	Idle			UMETA(DisplayName = "Idle"),
	Attack01		UMETA(DisplayName = "Attack01"),
	Attack02		UMETA(DisplayName = "Attack02"),
	Attack03		UMETA(DisplayName = "Attack03"),
	Die				UMETA(DisplayName = "Die")
};

UCLASS()
class GIGANTES_API AGtEliteEnemy : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGtEliteEnemy();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss")
	EBossState BossState;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss")
	AActor* FindEnemyActor;

	void ReAttack();

	float BossHP;
	float BossDamage;
	void GetDamage(float Damage);
	UFUNCTION()
	void Attack(UPrimitiveComponent* OverlappedComponent, 
			   AActor* OtherActor, 
			   UPrimitiveComponent* OtherComp, 
			   int32 OtherBodyIndex, 
			   bool bFromSweep, 
			   const FHitResult& SweepResult);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
						int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	void ChoiceAttack();
	
	void PlayerTraceTimerOn();
	void AttacktTmer01_On();
	void AttacktTmer02_On();
	void AttacktTmer03_On();
	void PlayerTrace();
	void Attackt01();
	void Attackt02();
	void Attackt03();
	

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy")
	USkeletalMeshComponent* SkeletalMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy|TracePlayer")
	USphereComponent* SphereCollision;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy|collision")
	UBoxComponent* HitBox_R;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy|collision")
	UBoxComponent* HitBox_L;

protected:
	FTimerHandle PlayerTraceTimer;
	FTimerHandle AttacktTmer01;
	FTimerHandle AttacktTmer02;
	FTimerHandle AttacktTmer03;

	TArray<int32> Patern;
	
};
