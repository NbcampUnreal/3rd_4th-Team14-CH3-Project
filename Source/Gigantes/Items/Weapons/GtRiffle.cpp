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

void AGtRiffle::OnFire()
{
	
}

void AGtRiffle::OnReload()
{
	
}