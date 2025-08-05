// Fill out your copyright notice in the Description page of Project Settings.

#include "GtEnemyAiController.h"
#include "GtEnemyMotherAiController.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGtEnemyAiController::AGtEnemyAiController()
{
	PrimaryActorTick.bCanEverTick = false;
	
	//ai
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerception);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.0f;
	SightConfig->LoseSightRadius = 1700.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	SightConfig->SetMaxAge(2.0f);

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
	
	//State
	EAiState CurrentState = EAiState::Idle;
	//
	MotherAiPosition = FVector::ZeroVector;
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

	SelectTimerChoice(EAiState::Random);
}

//ai
void AGtEnemyAiController::AiSelectItSelf()
{
	int select = FMath::RandRange(0, 1);
	if (select == 0)
	{
		EAiState CurrentState = EAiState::Random;
		SelectTimerChoice(CurrentState);
	}
	else
	{
		EAiState CurrentState = EAiState::MotherAi;
		SelectTimerChoice(CurrentState);
	}
}

void AGtEnemyAiController::GetMotherAiPosition(FVector CharPosition)
{
	MotherAiPosition = CharPosition;
}

//timer 
void AGtEnemyAiController::SelectTimerChoice(EAiState AiState)
{
	switch (AiState)
	{
	case EAiState::Idle:
		
		break;

	case EAiState::MotherAi:
		
		break;
		
	case EAiState::Random:
		RandomMoveTimerOn();
		break;

	case EAiState::Chase:
		ChaseTimerOn();
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
		//SelectTimerChoice(static_cast<EAiState>(255));  // default play
	}
}

void AGtEnemyAiController::RandomMoveTimerOn()
{
	GetWorldTimerManager().SetTimer(
	RandomMoveTimer,
	this,
	&AGtEnemyAiController::MoveToRandomLocation,
	3.0f,
	true,
	1.0f
	);
}

void AGtEnemyAiController::ChaseTimerOn()
{
	GetWorldTimerManager().SetTimer(
	ChaseTimer,
	this,
	&AGtEnemyAiController::UpdateChase,
	0.25f,
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
	GetWorldTimerManager().ClearTimer(RandomMoveTimer);
	GetWorldTimerManager().ClearTimer(ChaseTimer);
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
		UE_LOG(LogTemp, Warning, TEXT("[Sparta] Saw something! %s"), *Actor->GetName());

		DrawDebugString(
			GetWorld(),
			Actor->GetActorLocation() + FVector(0, 0, 100),
			FString::Printf(TEXT("Saw: %s"), *Actor->GetName()),
			nullptr,
			FColor::Green,
			2.0f,
			true
		);
	
		float Distance = GetPawn()->GetDistanceTo(CurrentTarget);
		if (Distance <= 1000.f)
		{
			StopChasing();
			AttackAction();
		}
		else
		{
			StartChasing(Actor);	
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Sparta] Missed it! %s"), *Actor->GetName());

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
	}
}

//movement
void AGtEnemyAiController::IdleAction()
{
	EAiState CurrentState = EAiState::Idle;
}

void AGtEnemyAiController::FollowMotherAiPlayerPosition()
{
	EAiState CurrentState = EAiState::MotherAi;
	UE_LOG(LogTemp, Warning, TEXT("받은 위치: %s"), *MotherAiPosition.ToString());

	APawn* MyPawn = GetPawn();
	if (!MyPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("[Sparta] No Pawn to control."));
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Sparta] Could not find Navigation System."));
	}
	
	MoveToLocation(MotherAiPosition);
}

void AGtEnemyAiController::MoveToRandomLocation()
{
	EAiState CurrentState = EAiState::Random;
	APawn* MyPawn = GetPawn();
	if (!MyPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("[Sparta] No Pawn to control."));
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Sparta] Could not find Navigation System."));
	}

	FNavLocation RandomLocation;
	bool bFoundLocation = NavSystem->GetRandomReachablePointInRadius(
		MyPawn->GetActorLocation(),
		MoveRadius,
		RandomLocation
	);

	if (bFoundLocation)
	{
		MoveToLocation(RandomLocation.Location);

		UE_LOG(LogTemp, Warning, TEXT("[Sparta] Move target: %s"), *RandomLocation.Location.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Sparta] Could not find a reachable location."));
	}
}

void AGtEnemyAiController::StartChasing(AActor* Target)
{
	if (bIsChasing && CurrentTarget == Target) return;

	CurrentTarget = Target;
	bIsChasing = true;

	ClearAllTimers();

	if (ATestAiCharacter* AIChar = Cast<ATestAiCharacter>(GetPawn()))
	{
		AIChar->SetMovementSpeed(AIChar->RunSpeed);
	}

	UpdateChase();
	EAiState CurrentState = EAiState::Chase;
	SelectTimerChoice(CurrentState);
}

void AGtEnemyAiController::UpdateChase()
{
	if (CurrentTarget && bIsChasing)
	{
		MoveToActor(CurrentTarget, 1000.0f);
	}
}

void AGtEnemyAiController::StopChasing()
{
	if (!bIsChasing) return;

	CurrentTarget = nullptr;
	bIsChasing = false;

	ClearAllTimers();
	StopMovement();

	if (ATestAiCharacter* AIChar = Cast<ATestAiCharacter>(GetPawn()))
	{
		AIChar->SetMovementSpeed(AIChar->WalkSpeed);
	}
}

void AGtEnemyAiController::AttackAction()
{
	ClearAllTimers();
	EAiState CurrentState = EAiState::Attack;
	
	
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
	EAiState CurrentState = EAiState::Reload;
	
	SelectTimerChoice(EAiState::Attack);
}