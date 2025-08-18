#pragma once
#include "CoreMinimal.h"
#include "Gigantes/Items/Structs/FGtItemData.h"
#include "GtItemInstance.generated.h"

UCLASS(BlueprintType)
class UGtItemInstance : public UObject {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly) FGtItemData Data;
	UPROPERTY(BlueprintReadOnly) int32 Count = 1;
	void Init(const FGtItemData& InData, int32 InCount=1) { Data=InData; Count=InCount; }
};