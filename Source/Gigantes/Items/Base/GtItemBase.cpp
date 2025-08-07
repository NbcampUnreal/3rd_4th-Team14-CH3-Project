#include "GtItemBase.h"

// Sets default values
AGtItemBase::AGtItemBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AGtItemBase::UseItem()
{
	UE_LOG(LogTemp, Warning, TEXT("Base item used: %s"), *ItemData.ItemName);
}

void AGtItemBase::InitFromData(const FGtItemData& InData)
{
	ItemData = InData;
}