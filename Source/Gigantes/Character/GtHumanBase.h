// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GtCharacterBase.h"
#include "GtHumanBase.generated.h"

UCLASS()
class GIGANTES_API AGtHumanBase : public AGtCharacterBase
{
	GENERATED_BODY()

public:
	AGtHumanBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

};
