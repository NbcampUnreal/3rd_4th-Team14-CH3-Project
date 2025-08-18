#pragma once
#include "CoreMinimal.h"
#include "GtItemId.generated.h"

USTRUCT(BlueprintType)
struct FGtItemId {
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Value;
	FGtItemId() {}
	explicit FGtItemId(const FString& In) : Value(In) {}
	bool IsValid() const { return !Value.IsEmpty(); }
	bool operator==(const FGtItemId& O) const { return Value == O.Value; }
};