#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GtGameStateBase.generated.h"

UCLASS()
class GIGANTES_API AGtGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Game")
	int32 CurrentScore = 0;  //현재 점수

	UPROPERTY(BlueprintReadOnly, Category = "Game")
	int32 RemainingEnemies = 10;  //남은적 갯수

	UPROPERTY(BlueprintReadOnly, Category = "Game")
	float ElapsedTime = 0.0f;  // 경과 시간 (UI 노출용 추가)
	
	UFUNCTION(BlueprintCallable, Category = "Game")
	int32 GetScore() const { return CurrentScore; }
};
