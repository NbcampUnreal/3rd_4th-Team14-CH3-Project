// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gigantes/Items/Base/GtItemBase.h"
#include "GtGranadeItem.generated.h"

UCLASS()
class GIGANTES_API AGtGranadeItem : public AGtItemBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGtGranadeItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
