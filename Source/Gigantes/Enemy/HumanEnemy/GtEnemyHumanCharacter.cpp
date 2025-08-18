// Fill out your copyright notice in the Description page of Project Settings.

#include "GtEnemyHumanCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GtEnemyAiController.h"
#include "Components/SphereComponent.h"


AGtEnemyHumanCharacter::AGtEnemyHumanCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	AIControllerClass = AGtEnemyAiController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = false;

	//rotate character to ahead
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);


}

void AGtEnemyHumanCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGtEnemyHumanCharacter::Die()
{
	Super::Die();
	
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->SetSimulatePhysics(true);

		AGtEnemyAiController* MyAIController = Cast<AGtEnemyAiController>(GetController());

		if (MyAIController)
		{
			MyAIController->Die();
		}	
	}
}

