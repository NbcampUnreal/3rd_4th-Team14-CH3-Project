#include "GtSniper.h"


// Sets default values
AGtSniper::AGtSniper()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGtSniper::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGtSniper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

