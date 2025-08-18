#pragma once

#include "CoreMinimal.h"
#include "GtGrenadeItem.h"
#include "GtNormal.generated.h"

UCLASS()
class GIGANTES_API AGtNormal : public AGtGrenadeItem
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGtNormal();

protected:
	virtual void Explode() override;
};
