#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gigantes/Items/Structs/FGtItemData.h"
#include "GtItemBase.generated.h"

UCLASS(Abstract)
class GIGANTES_API AGtItemBase : public AActor
{
	GENERATED_BODY()

public:
	AGtItemBase();
	
	virtual void InitFromData(const FGtItemData& data);
	virtual void UseItem();

	const FGtItemData& GetItemData() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item|Debug")
	FString ItemId;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item")
	FGtItemData ItemData = FGtItemData{};
};
