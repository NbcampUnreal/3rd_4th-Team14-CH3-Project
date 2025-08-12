#include "GtRifle.h"


// Sets default values
AGtRifle::AGtRifle()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Damage        = 28.f;
	FireRate  = 0.09f;  // 초당 ~11발
	ReloadTime    = 2.2f;
	MaxAmmo       = 30;
	AmmoInMagazine= 30;
	BulletSpreadDeg = 0.8f;
	TraceRange    = 10000.f;
}
