#pragma once

#include "CoreMinimal.h"
#include "GtBaseAnimInstance.h"
#include "GtHeroAnimInstance.generated.h"

class AGtHeroCharacter;

struct FGtHeroAnimInstanceProxy : public FGtBaseAnimInstanceProxy
{
	FGtHeroAnimInstanceProxy(UAnimInstance* Instance);
	
	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;
	
	float CachedAimOffsetYaw;
	float CachedAimOffsetPitch;
	float CachedGroundDistance;
};

UCLASS()
class GIGANTES_API UGtHeroAnimInstance : public UGtBaseAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "AimOffset")
	float AimOffsetPitch;
	
	UPROPERTY(BlueprintReadOnly, Category = "AimOffset")
	float AimOffsetYaw;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundDistance;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsWallRunning;
    
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsWallRunningRight;  // true = 오른쪽, false = 왼쪽

};
