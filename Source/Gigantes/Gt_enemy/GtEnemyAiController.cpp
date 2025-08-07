// Fill out your copyright notice in the Description page of Project Settings.

#include "GtEnemyAiController.h"
#include "GtEnemyMotherAiController.h"
#include "NavigationSystem.h"
#include "RewindData.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGtEnemyAiController::AGtEnemyAiController()
{
	PrimaryActorTick.bCanEverTick = false;
	
	//ai
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SetPerceptionComponent(*AIPerception);
	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
	
	SightConfig->SightRadius = 1500.0f;
	SightConfig->LoseSightRadius = 1700.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	SightConfig->SetMaxAge(2.0f);

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	
	MotherAiPosition = FVector::ZeroVector;
		
	//State
	EAiState CurrentState = EAiState::Move;
	ObeyValue = FMath::RandRange(1, 10);
}

void AGtEnemyAiController::BeginPlay()
{
	Super::BeginPlay();

	//delegate
	AGtEnemyMotherAiController* AI = Cast<AGtEnemyMotherAiController>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AGtEnemyMotherAiController::StaticClass()));

	if (AI)
	{
		AI->OnPlayerPositionFound.AddDynamic(this, &AGtEnemyAiController::GetMotherAiPosition);
	}
	
	if (AIPerception)
	{
		AIPerception->OnTargetPerceptionUpdated.AddDynamic(
			this,
			&AGtEnemyAiController::OnPerceptionUpdated
		);
	}

	MoveToTimerOn();
}

void AGtEnemyAiController::GetMotherAiPosition(FVector CharPosition)
{
	MotherAiPosition = CharPosition;
	//UE_LOG(LogTemp, Warning, TEXT("Player Position : %f, %f, %f"), MotherAiPosition.X, MotherAiPosition.Y, MotherAiPosition.Z);
}

//timer 
void AGtEnemyAiController::SelectTimerChoice(EAiState AiState)
{
	switch (AiState)
	{
	case EAiState::Idle:
		
		break;
		
	case EAiState::Move:
		MoveToTimerOn();
		break;

	case EAiState::Attack:
		AttackTimerOn();
		break;

	case EAiState::Reload:
		ReloadTimerOn();
		break;

	default:
		// Optional: Handle unknown states
		break;
		//SelectTimerChoice(static_cast<EAiState>(255));  // when default play
	}
}

void AGtEnemyAiController::MoveToTimerOn()
{
	GetWorldTimerManager().SetTimer(
	MoveToTimer,
	this,
	&AGtEnemyAiController::MoveRandomOrAiLocation,
	4.0f,
	true
	);
}

void AGtEnemyAiController::AttackTimerOn()
{
	GetWorldTimerManager().SetTimer(
	AttackTimer,
	this,
	&AGtEnemyAiController::AttackAction,
	0.25f,
	false
	);
}

void AGtEnemyAiController::ReloadTimerOn()
{
	GetWorldTimerManager().SetTimer(
	ReloadTimer,
	this,
	&AGtEnemyAiController::ReloadAction,
	0.25f,
	false,
	1.0f
	);
}

void AGtEnemyAiController::ClearAllTimers()
{
	GetWorldTimerManager().ClearTimer(MoveToTimer);
	GetWorldTimerManager().ClearTimer(AttackTimer);
	GetWorldTimerManager().ClearTimer(ReloadTimer);
}

//perception
void AGtEnemyAiController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Actor != PlayerPawn)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAi] Saw something! %s"), *Actor->GetName());

		DrawDebugString(
			GetWorld(),
			Actor->GetActorLocation() + FVector(0, 0, 100),
			FString::Printf(TEXT("Saw: %s"), *Actor->GetName()),
			nullptr,
			FColor::Green,
			2.0f,
			true
		);

		StartChasing(Actor);
		
		float Distance = GetPawn()->GetDistanceTo(CurrentTarget);
		if (Distance <= 1000.f)
		{
			//StopChasing();
			//AttackAction();
		}
		else
		{
			
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAi] Missed it! %s"), *Actor->GetName());

		DrawDebugString(
			GetWorld(),
			Actor->GetActorLocation() + FVector(0, 0, 100),
			FString::Printf(TEXT("Missed: %s"), *Actor->GetName()),
			nullptr,
			FColor::Red,
			2.0f,
			true
		);
		StopChasing();
		MoveRandomOrAiLocation();
	}
}

//movement
void AGtEnemyAiController::IdleAction()
{
	EAiState CurrentState = EAiState::Idle;
}

void AGtEnemyAiController::MoveRandomOrAiLocation()
{
	EAiState CurrentState = EAiState::Move;
	APawn* MyPawn = GetPawn();
	if (!MyPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("[EnemyAi] No Pawn to control."));
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[EnemyAi] Could not find Navigation System."));
	}

	FNavLocation RandomLocation;
	bool bFoundLocation = NavSystem->GetRandomReachablePointInRadius(
		MyPawn->GetActorLocation(),
		MoveRadius,
		RandomLocation
	);
	
	if (bFoundLocation)
	{
	int select = FMath::RandRange(1, 10);
		//UE_LOG(LogTemp, Warning, TEXT("[EnemyAi] Ai Choice %f, %f "), *select, *ObeyValue);
		if (select < ObeyValue)
		{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAi] select :%i, ObeyValue : %i."), select, ObeyValue);
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAi] Move to Around player."));
			
		// position z overspace covering
		FNavLocation ProjectedLocation;
		bool bProjected = NavSystem->ProjectPointToNavigation(
			MotherAiPosition,
			ProjectedLocation,
			FVector(100.0f, 100.0f, 500.0f) // 탐색 반경 (Z 500은 공중 커버)
			);
			if (bProjected)
			{
			MoveToLocation(ProjectedLocation.Location);
			UE_LOG(LogTemp, Warning, TEXT("[EnemyAi] Move target (Projected): %s"), *ProjectedLocation.Location.ToString());
			}
			else
			{
			UE_LOG(LogTemp, Error, TEXT("[EnemyAi] MotherAiPosition is NOT on NavMesh! Raw: %s"), *MotherAiPosition.ToString());
			}

			UE_LOG(LogTemp, Warning, TEXT("[EnemyAi] Move target: X: %f, Y: %f, Z: %f"), MotherAiPosition.X,MotherAiPosition.Y, MotherAiPosition.Z );
		}
		
		else
		{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAi] select :%i, ObeyValue : %i."), select, ObeyValue);
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAi] Move to RandomPosition."));
		MoveToLocation(RandomLocation.Location);
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAi] Move target: %s"), *RandomLocation.Location.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAi] Could not find a reachable location."));
	}
}

void AGtEnemyAiController::StartChasing(AActor* Target)
{
	EAiState CurrentState = EAiState::Chase;
	
	if (bIsChasing && CurrentTarget == Target) return;
	CurrentTarget = Target;
	bIsChasing = true;

	GetWorldTimerManager().ClearTimer(MoveToTimer);
/*
	if (ATestAiCharacter* AIChar = Cast<ATestAiCharacter>(GetPawn()))
	{
		AIChar->SetMovementSpeed(AIChar->RunSpeed);
	}
*/
	UpdateChase();
}

void AGtEnemyAiController::UpdateChase()
{
	if (!CurrentTarget) return;

	APawn* MyPawn = GetPawn();
	if (!MyPawn) return;

	if (bIsChasing)
	{
		MoveToActor(CurrentTarget, 300.0f);
	}

	float Distance = FVector::Dist(MyPawn->GetActorLocation(), CurrentTarget->GetTargetLocation());

	if (Distance < 400.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAi] Close enough to attack. Distance: %f"), Distance);
		AttackTimerOn();
	}
}


void AGtEnemyAiController::StopChasing()
{
	if (!bIsChasing) return;
	MoveToLocation(CurrentTarget->GetTargetLocation());
	CurrentTarget = nullptr;
	bIsChasing = false;

	ClearAllTimers();
	StopMovement();
	MoveToTimerOn();
/*
	if (ATestAiCharacter* AIChar = Cast<ATestAiCharacter>(GetPawn()))
	{
		AIChar->SetMovementSpeed(AIChar->WalkSpeed);
	}
*/
}

void AGtEnemyAiController::AttackAction()
{
	EAiState CurrentState = EAiState::Attack;
	UE_LOG(LogTemp, Warning, TEXT("[EnemyAi] Shoot Player."));
	ClearAllTimers();
	
	FVector Direction = CurrentTarget->GetActorLocation() - GetPawn()->GetActorLocation();
	Direction.Z = 0;
    FRotator TargetRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
	
	GetPawn()->SetActorRotation(TargetRotation);
	
	//attack to player
	
	SelectTimerChoice(EAiState::Reload);
}

void AGtEnemyAiController::ReloadAction()
{	
	ClearAllTimers();
	UE_LOG(LogTemp, Warning, TEXT("[EnemyAi] Reloading."));
	EAiState CurrentState = EAiState::Reload;

	if (CurrentTarget != nullptr)
	{
		SelectTimerChoice(EAiState::Attack);
	}
	else
	{
		SelectTimerChoice(EAiState::Move);
	}
	
}