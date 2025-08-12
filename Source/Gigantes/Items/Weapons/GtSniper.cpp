#include "GtSniper.h"


// Sets default values
AGtSniper::AGtSniper()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Damage        = 80.f;
	FireRate  = 1.2f;
	ReloadTime    = 3.0f;
	MaxAmmo       = 5;
	AmmoInMagazine= 5;
	BulletSpreadDeg = 0.1f;
	TraceRange    = 20000.f;
}


