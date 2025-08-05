// Fill out your copyright notice in the Description page of Project Settings.


#include "GtEnemyMotherAiController.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AGtEnemyMotherAiController::AGtEnemyMotherAiController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
}

// Called when the game starts or when spawned
void AGtEnemyMotherAiController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("MotherAI Wake up"));
	GetWorldTimerManager().SetTimer(
		FindPlayerHandle,
		this,
		&AGtEnemyMotherAiController::FindPlayerPosition,
		1.0f,
		true
		);
	
	
}

void AGtEnemyMotherAiController::FindPlayerPosition()
{
	FVector CharPositon = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorLocation();
		
	FVector NewPosition(CharPositon.X+RandomNumber(), CharPositon.Y+RandomNumber(), CharPositon.Z);

	//UE_LOG(LogTemp, Warning, TEXT("Player Position : %f, %f, %f"), NewPosition.X, NewPosition.Y, NewPosition.Z);
	OnPlayerPositionFound.Broadcast(NewPosition);
}

int AGtEnemyMotherAiController::RandomNumber()
{
	int Range = 1000; //단계별로 저 정밀하게
	return FMath::RandRange(-Range, Range);
	
}