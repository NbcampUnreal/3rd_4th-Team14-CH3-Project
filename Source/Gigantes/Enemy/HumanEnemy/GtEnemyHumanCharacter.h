// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gigantes/Character/GtHumanBase.h"
#include "GtEnemyHumanCharacter.generated.h"

UCLASS()
class GIGANTES_API AGtEnemyHumanCharacter : public AGtHumanBase
{
	GENERATED_BODY()

public:
	AGtEnemyHumanCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Die() override;
};
