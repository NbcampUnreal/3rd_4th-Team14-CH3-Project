// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GtWeaponItem.h"
#include "GtHandGun.generated.h"

UCLASS()
class GIGANTES_API AGtHandGun : public AGtWeaponItem
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGtHandGun();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
