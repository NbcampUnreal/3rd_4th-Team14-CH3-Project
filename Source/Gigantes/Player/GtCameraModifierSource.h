#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GtCameraModifierSource.generated.h"

struct FGtCameraModifier;
struct FGameplayTag;

// This class does not need to be modified.
UINTERFACE()
class UGtCameraModifierSource : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GIGANTES_API IGtCameraModifierSource
{
	GENERATED_BODY()

public:
	// 이 객체가 특정 상태 태그에 대한 카메라 모디파이어를 가지고 있는지 확인하고 있다면 반환
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera")
	bool GetCameraModifierForTag(const FGameplayTag& ActionTag, FGtCameraModifier& OutModifier) const;
};
