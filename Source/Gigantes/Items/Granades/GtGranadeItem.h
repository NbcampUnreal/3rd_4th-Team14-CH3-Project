#pragma once

#include "CoreMinimal.h"
#include "Gigantes/Items/Base/GtItemBase.h"
#include "GtGranadeItem.generated.h"

UCLASS()
class GIGANTES_API AGtGranadeItem : public AGtItemBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGtGranadeItem();

	virtual void Throw();

protected:
	virtual void Explode();

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float ExplosionDelay = 3.0f;
	
	FTimerHandle ExplosionTimer;
};
