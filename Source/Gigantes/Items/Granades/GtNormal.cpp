#include "GtNormal.h"


// Sets default values
AGtNormal::AGtNormal()
{
	ExplosionDelay = 2.5f;
}

void AGtNormal::Explode()
{
	Super::Explode();

	UE_LOG(LogTemp, Log, TEXT("[Grenade] Explode"));
}
