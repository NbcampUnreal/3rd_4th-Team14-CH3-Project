// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GtGranadeItem.h"
#include "GtNormal.generated.h"

UCLASS()
class GIGANTES_API AGtNormal : public AGtGranadeItem
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGtNormal();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
