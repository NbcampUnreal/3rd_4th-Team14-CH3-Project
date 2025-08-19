#include "GtItemBase.h"

// Sets default values
AGtItemBase::AGtItemBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

bool AGtItemBase::UseItem(AActor* User)
{
	UE_LOG(LogTemp, Warning, TEXT("[Base] user used: %p"), User);
	return false;
}

const FGtItemData& AGtItemBase::GetItemData() const
{
	return ItemData;
}

void AGtItemBase::InitFromData(const FGtItemData& InData)
{
	ItemData = InData;
	ItemId   = InData.ItemId; 
}
