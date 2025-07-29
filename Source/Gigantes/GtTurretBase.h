// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "GtTurretBase.generated.h"


UCLASS()
class GIGANTES_API AGtTurretBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGtTurretBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	class USkeletalMeshComponent* TurretMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	USphereComponent* Collision;

	void EnermySearch();
	FTimerHandle FindEnermyHandle;
	bool bIsFindEnermy;
	
	AActor* FindEnemyActor;

	UFUNCTION()
	void LookAt(AActor* Target);
	
public:	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
					   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
					   bool bFromSweep, const FHitResult& SweepResult);
    
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
					 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
};
