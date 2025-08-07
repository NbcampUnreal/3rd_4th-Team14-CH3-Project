#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Structs/FGtItemData.h"
#include "GtItemBase.generated.h"

UCLASS()
class GIGANTES_API AGtItemBase : public AActor
{
	GENERATED_BODY()

protected:
	FGtItemData ItemData;
	
public:
	AGtItemBase();
	virtual void InitFromData(const FGtItemData& data);
	virtual void UseItem();

	const FGtItemData& GetItemData() const;
	FString GetItemName() const;
	FString GetItemId() const;
	FString GetItemType() const;
	FGameplayTag GetItemTag() const;
	
};
