// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GtEnemyMotherAiController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerPositionFound, FVector, CharPosition);

UCLASS()
class GIGANTES_API AGtEnemyMotherAiController : public AAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGtEnemyMotherAiController();

	//Deligate
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnPlayerPositionFound OnPlayerPositionFound;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//timer
	FTimerHandle FindPlayerHandle;
	
	void FindPlayerPosition();
	int RandomNumber();
};
