#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gigantes/Items/Structs/FGtItemData.h"
#include "GtItemBase.generated.h"

UCLASS()
class GIGANTES_API AGtItemBase : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere)
	FGtItemData ItemData;
	
public:
	AGtItemBase();
	virtual void InitFromData(const FGtItemData& data);
	virtual bool UseItem();

	const FGtItemData& GetItemData() const;
	
};
