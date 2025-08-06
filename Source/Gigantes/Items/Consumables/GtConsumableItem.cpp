// Fill out your copyright notice in the Description page of Project Settings.


#include "GtConsumableItem.h"


// Sets default values
AGtConsumableItem::AGtConsumableItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGtConsumableItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGtConsumableItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

