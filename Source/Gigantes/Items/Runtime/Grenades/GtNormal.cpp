#include "GtNormal.h"


// Sets default values
AGtNormal::AGtNormal()
{
	FuseSeconds  = 0.5f;
	DamageRadius = 400.f;
	Damage       = 100.f;
}

void AGtNormal::Explode()
{
	Super::Explode();

	UE_LOG(LogTemp, Log, TEXT("[Grenade] Explode"));
}
