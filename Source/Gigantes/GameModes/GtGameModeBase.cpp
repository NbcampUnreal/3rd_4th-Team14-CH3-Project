
#include "GtGameModeBase.h"
#include "GtGameStateBase.h"
#include "Blueprint/UserWidget.h"  // UMG 위젯 사용
#include "Kismet/GameplayStatics.h"

AGtGameModeBase::AGtGameModeBase()
{
	GameStateClass = AGtGameStateBase::StaticClass();
}

void AGtGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Game Started!"));

	check(GEngine != nullptr);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Game Started!"));

	// 초기화 강화: 변수 리셋
	CurrentEnemiesKilled = 0;
	ElapsedTime = 0.0f;

	if (AGtGameStateBase* GS = GetGameState<AGtGameStateBase>())
	{
		GS->RemainingEnemies = MaxEnemies;
		GS->CurrentScore = 0;
		GS->ElapsedTime = 0.0f;
	}
	
	if (UClass* MenuClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/WBP_GtMainMenu.WBP_GtMainMenu_C")))
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			UUserWidget* MenuWidget = CreateWidget<UUserWidget>(PC, MenuClass);
			if (MenuWidget)
			{
				MenuWidget->AddToViewport();  // 뷰포트에보이게
				UGameplayStatics::SetGamePaused(GetWorld(), true);  // 일시정지
				PC->SetShowMouseCursor(true);  // 마우스 커서 보이게
				PC->SetInputMode(FInputModeUIOnly());  // UI입력모드
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to create WBP_MainMenu widget!"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PlayerController not found!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load WBP_MainMenu class!"));
	}

	if (HUDWidgetClass)
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			UUserWidget* HUD = CreateWidget<UUserWidget>(PC, HUDWidgetClass);
			if (HUD)
			{
				HUD->AddToViewport();  // HUD 뷰포트에 추가 
			}
		}
	}
	// 초기화: 적 스폰
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
	
	// UI 전환 확장: 결과 화면 위젯 표시 (UMG 예시)
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
