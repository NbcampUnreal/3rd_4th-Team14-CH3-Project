// Fill out your copyright notice in the Description page of Project Settings.

#include "GtEnemyHumanCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GtEnemyAiController.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"  // UGameplayStatics 사용 위해
#include "Gigantes/GameModes/GtGameModeBase.h"


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

	// GameMode 가져와서 EnemyKilled 호출 (헤드샷 아니면 false)
	if (UWorld* World = GetWorld())
	{
		AGtGameModeBase* GameMode = Cast<AGtGameModeBase>(UGameplayStatics::GetGameMode(World));
		if (GameMode)
		{
			GameMode->EnemyKilled(false);  // bHeadshot = false (필요 시 데미지 이벤트에서 true 전달)
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode not found in Die()!"));
		}
	}
}

