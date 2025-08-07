// Fill out your copyright notice in the Description page of Project Settings.


#include "GtRiffle.h"


// Sets default values
AGtRiffle::AGtRiffle()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AGtRiffle::InitFromData(const FGtItemData& InData)
{
	Super::InitFromData(InData);
}

void AGtRiffle::Fire()
{
	UE_LOG(LogTemp, Warning, TEXT("[Riffle] Fire! Rate: %f"), FireRate);
}

void AGtRiffle::Reload()
{
	UE_LOG(LogTemp, Warning, TEXT("[Riffle] Reloading..."));
}