// Fill out your copyright notice in the Description page of Project Settings.


#include "GtEnemyMotherAiController.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AGtEnemyMotherAiController::AGtEnemyMotherAiController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	PastPlayerPosition = FVector::ZeroVector;
}

// Called when the game starts or when spawned
void AGtEnemyMotherAiController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("MotherAI Wake up"));
	FindPlayerPosition();
	GetWorldTimerManager().SetTimer(
		FindPlayerHandle,
		this,
		&AGtEnemyMotherAiController::FindPlayerPosition,
		3.0f,
		true
		);
}

void AGtEnemyMotherAiController::FindPlayerPosition()
{
	if (FVector::ZeroVector != PastPlayerPosition)
	{
		//send almost correct player position.
		OnPlayerPositionFound.Broadcast(PastPlayerPosition);	
	}
	//make not clear player position data
	FVector CharPositon = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorLocation();
	FVector NewPosition(CharPositon.X+RandomNumber(),CharPositon.Y+RandomNumber(), CharPositon.Z);
	PastPlayerPosition = NewPosition;
	//UE_LOG(LogTemp, Warning, TEXT("Player Position : %f, %f, %f"), NewPosition.X, NewPosition.Y, NewPosition.Z);
}

int AGtEnemyMotherAiController::RandomNumber()
{
	//Be highlevel to low value
	int Range = 500; 
	return FMath::RandRange(-Range, Range);
}