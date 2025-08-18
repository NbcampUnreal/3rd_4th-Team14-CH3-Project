#pragma once
#include "CoreMinimal.h"
#include "GtItemId.h"
#include "GtItemStack.generated.h"

USTRUCT(BlueprintType)
struct FGtItemStack {
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGtItemId ItemId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Count = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool  bEquipped = false;
};