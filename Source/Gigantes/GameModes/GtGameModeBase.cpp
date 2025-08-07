#include "GtGameModeBase.h"
#include "GtGameStateBase.h"
#include "Kismet/GameplayStatics.h"

AGtGameModeBase::AGtGameModeBase()
{
	GameStateClass = AGtGameStateBase::StaticClass();
	// PlayerControllerClass = ... (캐릭터 팀원 지정)
	// HUDClass = AGtHUD::StaticClass(); (UI 팀원 작업 후 추가)
}

void AGtGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Game Started!"));

	check(GEngine != nullptr);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Game Started!"));
	// 초기화: 적 스폰, UI 표시 등
}

void AGtGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bGameOver || bGameCleared) return;

	ElapsedTime += DeltaTime;
	if (ElapsedTime >= GameTimeLimit)
	{
		EndGame(false);  //시간초과되면 게임종료
	}
}

void AGtGameModeBase::AddScore(int32 Points)
{
	if (AGtGameStateBase* GS = GetGameState<AGtGameStateBase>())
	{
		GS->CurrentScore += Points;  //점수추가
		//헤드샷하면 추가점수를 구현?(AI/캐릭터 팀원과 협의)
	}
}

void AGtGameModeBase::EnemyKilled()
{
	CurrentEnemiesKilled++;
	AddScore(100);  //기본득점

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
	//UI구현(Blueprint 이벤트나 UMG Widget 생성)
	//예: UGameplayStatics::OpenLevel(GetWorld(), "MainMenu");
}
