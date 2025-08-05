// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"
#include "AIController.h"
#include "GtEnemyAiController.generated.h"

//enum state
	UENUM(BlueprintType)
enum class EAiState : uint8
	{
		Idle       UMETA(DisplayName = "Idle"),
		MotherAi   UMETA(DisplayName = "MotherAi"),
		Random     UMETA(DisplayName = "Random"),
		Chase      UMETA(DisplayName = "Chase"),
		Attack     UMETA(DisplayName = "Attack"),
		Reload     UMETA(DisplayName = "Reload")
	};

UCLASS()
class GIGANTES_API AGtEnemyAiController : public AAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGtEnemyAiController();

protected:
	virtual void BeginPlay() override;
	//ai
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* AIPerception;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Sight* SightConfig;
	UPROPERTY(EditAnywhere, Category = "AI")
	float MoveRadius = 1000.0f;
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	void AiSelectItSelf();
	virtual void OnPossess(APawn* InPawn) override;
	UPROPERTY()
	AActor* CurrentTarget = nullptr;
	FVector MotherAiPosition;
	
	//timer
	bool bIsChasing = false;
	void SelectTimerChoice(EAiState AiState);
	FTimerHandle ChaseTimer;
	FTimerHandle RandomMoveTimer;
	FTimerHandle AttackTimer;
	FTimerHandle ReloadTimer;
	void RandomMoveTimerOn();
	void ChaseTimerOn();
	void AttackTimerOn();
	void ReloadTimerOn();
	//timer_clear
	void ClearAllTimers();

	//movement-idle
	void IdleAction();
	void GetMotherAiPosition(FVector CharPosition);
	//movement-follow_mother_ai
	UFUNCTION()
	void FollowMotherAiPlayerPosition();
	//movement-Random
	void MoveToRandomLocation();
	//movement-chase
	void StartChasing(AActor* Target);
	void UpdateChase();
	void StopChasing();
	//movement-attack
	void AttackAction();
	void ReloadAction();
	
};
