#include "GtGranadeItem.h"


// Sets default values
AGtGranadeItem::AGtGranadeItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AGtGranadeItem::Throw()
{
	GetWorld()->GetTimerManager().SetTimer(
		ExplosionTimer,
		this,
		&AGtGranadeItem::Explode,
		ExplosionDelay,
		false
	);

	UE_LOG(LogTemp, Log, TEXT("[Grenade] Throw %.2f seconds"), ExplosionDelay);
}

void AGtGranadeItem::Explode()
{
	UE_LOG(LogTemp, Warning, TEXT("[Grenade] BOOM!"));

	Destroy(); // 기본 폭발은 그냥 제거만
}
