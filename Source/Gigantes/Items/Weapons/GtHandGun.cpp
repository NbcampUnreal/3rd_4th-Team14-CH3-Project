#include "GtHandGun.h"


// Sets default values
AGtHandGun::AGtHandGun()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Damage        = 20.f;
	FireRate  	  = 0.25f;  // 초당 4발 정도
	ReloadTime    = 1.5f;
	MaxAmmo       = 15;
	AmmoInMagazine= 15;
	BulletSpreadDeg = 1.2f;
	TraceRange    = 9000.f;
}

