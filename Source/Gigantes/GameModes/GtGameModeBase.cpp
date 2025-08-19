
#include "GtGameModeBase.h"
#include "GtGameStateBase.h"
#include "Blueprint/UserWidget.h"  // UMG 위젯 사용
#include "Gigantes/Enemy/EliteEnemy/GtEliteEnemy.h"
#include "Gigantes/Enemy/HumanEnemy/GtEnemyHumanCharacter.h"
#include "Gigantes/Enemy/Turret/GtTurretBase.h"
#include "Kismet/GameplayStatics.h"

AGtGameModeBase::AGtGameModeBase()
{
	GameStateClass = AGtGameStateBase::StaticClass();
	PrimaryActorTick.bCanEverTick = true;
}

void AGtGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Game Started!"));

	check(GEngine != nullptr);

	CurrentEnemiesKilled = 0;
	ElapsedTime = 0.0f;

	TArray<AActor*> FoundEnemies;
	int32 TotalEnemyCount = 0;

	// 1. GtEnemyHumanCharacter 카운트
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGtEnemyHumanCharacter::StaticClass(), FoundEnemies);
	TotalEnemyCount += FoundEnemies.Num();

	// 2. GtEliteEnemy 카운트
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGtEliteEnemy::StaticClass(), FoundEnemies);
	TotalEnemyCount += FoundEnemies.Num();

	// 3. GtTurretBase 카운트
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGtTurretBase::StaticClass(), FoundEnemies);
	TotalEnemyCount += FoundEnemies.Num();

	MaxEnemies = TotalEnemyCount;
	
	if (AGtGameStateBase* GS = GetGameState<AGtGameStateBase>())
	{
		GS->RemainingEnemies = TotalEnemyCount;
		GS->CurrentScore = 0;
		GS->ElapsedTime = 0.0f;
	}
}

void AGtGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ElapsedTime += DeltaTime;
	if (AGtGameStateBase* GS = GetGameState<AGtGameStateBase>())
	{
		GS->ElapsedTime = ElapsedTime;// GS 동기화
	}
	if (ElapsedTime >= GameTimeLimit)
	{
		EndGame(false);
	}
}

void AGtGameModeBase::AddScore(int32 Points,bool bHeadshot)
{
	if (AGtGameStateBase* GS = GetGameState<AGtGameStateBase>())
	{
		GS->CurrentScore += Points;  //점수추가
		if (bHeadshot)
		{
			GS->CurrentScore += 50;  // 헤드샷 보너스 추가
		}
		OnScoreUpdated.Broadcast(GS->CurrentScore);  // 이벤트 디스패치 (UI 업데이트)
	}
}

void AGtGameModeBase::EnemyKilled(bool bHeadshot)
{
	CurrentEnemiesKilled++;
	AddScore(100, bHeadshot);  //기본득점+ 헤드샷체크

	if (AGtGameStateBase* GS = GetGameState<AGtGameStateBase>())
	{
		GS->RemainingEnemies--;  //남은적수 감소(UI 표시)
	}

	if (CurrentEnemiesKilled >= MaxEnemies)
	{
		EndGame(true);  //클리어시 게임종료
	}
}

void AGtGameModeBase::PlayerDied()
{
	EndGame(false);  //사망시 게임종료
}

void AGtGameModeBase::EndGame(bool bWon)
{
	bGameOver = !bWon;
	bGameCleared = bWon;
	UE_LOG(LogTemp, Warning, TEXT("%s"), bWon ? TEXT("Game Cleared!") : TEXT("Game Over!"));

	if (UClass* ResultClass = LoadClass<UUserWidget>(nullptr, bWon ? TEXT("/Game/UI/WBP_GtClear.WBP_GtClear_C") : TEXT("/Game/UI/WBP_GtGameOver.WBP_GtGameOver_C")))
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			UUserWidget* ResultWidget = CreateWidget<UUserWidget>(PC, ResultClass);
			if (ResultWidget)
			{
				ResultWidget->AddToViewport();
				UGameplayStatics::SetGamePaused(GetWorld(), true);
				PC->SetShowMouseCursor(true);
				PC->SetInputMode(FInputModeUIOnly());
			}
		}
	}
}
