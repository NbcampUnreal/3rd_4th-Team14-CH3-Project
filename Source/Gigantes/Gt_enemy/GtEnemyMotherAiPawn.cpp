// Fill out your copyright notice in the Description page of Project Settings.


#include "GtEnemyMotherAiPawn.h"


// Sets default values
AGtEnemyMotherAiPawn::AGtEnemyMotherAiPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	AIControllerClass = AGtEnemyMotherAiPawn::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void AGtEnemyMotherAiPawn::BeginPlay()
{
	Super::BeginPlay();
}

