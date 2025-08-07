#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GtGameModeBase.generated.h"

UCLASS()
class GIGANTES_API AGtGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGtGameModeBase();

	virtual void BeginPlay() override;  //게임시작
	virtual void Tick(float DeltaTime) override;  //틱

	UFUNCTION(BlueprintCallable, Category = "Game")
	void AddScore(int32 Points);  //점수추가
	
	UFUNCTION(BlueprintCallable, Category = "Game")
	void EnemyKilled();  //적처치시

	UFUNCTION(BlueprintCallable, Category = "Game")
	void PlayerDied();  //플레이어 사망시

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	int32 MaxEnemies = 10;  //클리어에 필요한 적의 갯수

	UPROPERTY(EditDefaultsOnly, Category = "Game")
	float GameTimeLimit = 300.0f;  //시간초과로 게임오버되는 제한시간

	int32 CurrentEnemiesKilled = 0; //현재처치한적갯수
	float ElapsedTime = 0.0f; //경과시간

	bool bGameOver = false;
	bool bGameCleared = false;

	void EndGame(bool bWon);  //게임종료
};
