#include "GtHumanBase.h"

AGtHumanBase::AGtHumanBase(const FObjectInitializer & ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
}
