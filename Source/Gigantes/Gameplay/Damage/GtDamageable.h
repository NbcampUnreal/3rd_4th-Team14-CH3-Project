#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamageSystemTypes.h"
#include "GtDamageable.generated.h"

UINTERFACE()
class UGtDamageable : public UInterface
{
	GENERATED_BODY()
};

class GIGANTES_API IGtDamageable
{
	GENERATED_BODY()

public:

	// 데미지 적용 인터페이스 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	bool ApplyDamage(const FGtDamageInfo& DamageInfo, FGtDamageResult& OutDamageResult);
};
