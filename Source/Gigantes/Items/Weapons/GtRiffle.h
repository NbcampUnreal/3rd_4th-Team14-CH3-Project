#pragma once

#include "CoreMinimal.h"
#include "GtWeaponItem.h"
#include "GtRiffle.generated.h"

UCLASS()
class GIGANTES_API AGtRiffle : public AGtWeaponItem
{
	GENERATED_BODY()

public:
	AGtRiffle();
	
	virtual void InitFromData(const FGtItemData& data) override;
	virtual void OnFire() override;
	virtual void OnReload() override;

};
