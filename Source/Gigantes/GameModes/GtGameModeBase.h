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
	void AddScore(int32 Points, bool bHeadshot = false);  //점수추가(헤드샷 파라미터 추가)
	
	UFUNCTION(BlueprintCallable, Category = "Game")
	void EnemyKilled(bool bHeadshot = false);  //적처치시(헤드샷 파라미터 추가)

	UFUNCTION(BlueprintCallable, Category = "Game")
	void PlayerDied();  //플레이어 사망시

protected:
	
	// Delegate 선언 (헤더에 추가)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdated, int32, NewScore);

	// 추가: 체력 변화 델리게이트 (UI 바인딩용)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerHealthChanged, float, OldHealth, float, NewHealth);
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnPlayerHealthChanged OnPlayerHealthChanged;

	// 추가: 총알 변화 델리게이트 (UI 바인딩용)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerAmmoChanged, int32, NewAmmo);
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnPlayerAmmoChanged OnPlayerAmmoChanged;
	
	// 이벤트 디스패처 추가 (UI 업데이트용, 블루프린트에서 바인딩 가능)
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnScoreUpdated OnScoreUpdated;  // 점수 업데이트 이벤트 (Delegate 추가 예시)
	
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	int32 MaxEnemies = 10;  //클리어에 필요한 적의 갯수

	UPROPERTY(EditDefaultsOnly, Category = "Game")
	float GameTimeLimit = 300.0f;  //시간초과로 게임오버되는 제한시간

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> HUDWidgetClass;
	
	int32 CurrentEnemiesKilled = 0; //현재처치한적갯수
	float ElapsedTime = 0.0f; //경과시간

	bool bGameOver = false;
	bool bGameCleared = false;

	void EndGame(bool bWon);  //게임종료

	

};
