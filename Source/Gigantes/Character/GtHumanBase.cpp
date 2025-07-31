#include "GtHumanBase.h"

AGtHumanBase::AGtHumanBase(FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
}
