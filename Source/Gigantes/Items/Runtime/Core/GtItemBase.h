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
	
	UFUNCTION(BlueprintCallable, Category="Item")
	virtual bool UseItem(AActor* User);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item|Debug")
	FString ItemId;
	
	const FGtItemData& GetItemData() const;

	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item")
	FGtItemData ItemData = FGtItemData{};
};
