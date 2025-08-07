#include "GtHeroAnimInstance.h"

#include "Gigantes/GtGameplayTags.h"
#include "Gigantes/Character/GtHeroCharacter.h"
#include "Gigantes/Character/Components/GtHeroMovementComponent.h"
#include "Gigantes/Player/GtPlayerCameraManager.h"

FGtHeroAnimInstanceProxy::FGtHeroAnimInstanceProxy(UAnimInstance* Instance)
	: FGtBaseAnimInstanceProxy(Instance) 
{
}

void FGtHeroAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FGtBaseAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);

	if (!InAnimInstance) return;
	
	AGtHeroCharacter* OwningHeroCharacter = Cast<AGtHeroCharacter>(InAnimInstance->GetOwningActor());
	if (OwningHeroCharacter)
	{
		if (APlayerController* PC = Cast<APlayerController>(OwningHeroCharacter->GetController()))
		{
			if (AGtPlayerCameraManager* CameraManager = Cast<AGtPlayerCameraManager>(PC->PlayerCameraManager))
			{
				CachedAimOffsetYaw = CameraManager->GetAimOffsetYaw();
				CachedAimOffsetPitch = CameraManager->GetAimOffsetPitch();
			}
			else
			{
				CachedAimOffsetYaw = 0.0f;
				CachedAimOffsetPitch = 0.0f;
			}
		}
		if (UGtHeroMovementComponent* HeroMovementComponent = Cast<UGtHeroMovementComponent>(OwningHeroCharacter->GetCharacterMovement()))
		{
			CachedGroundDistance = HeroMovementComponent->GetGroundDistance();
		}
	}
}

FAnimInstanceProxy* UGtHeroAnimInstance::CreateAnimInstanceProxy()
{
	return new FGtHeroAnimInstanceProxy(this);
}

void UGtHeroAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy)
{
	delete InProxy;
}

void UGtHeroAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	const auto& HeroAnimProxy = GetProxyOnAnyThread<FGtHeroAnimInstanceProxy>();

	bIsWallRunning = StatusTags.HasTag(GtGameplayTags::Status_Action_WallRunning);
	bIsWallRunningRight = StatusTags.HasTag(GtGameplayTags::Status_Action_WallRunning_Right);
	bIsCrouching = StatusTags.HasTag(GtGameplayTags::Status_Action_Crouching);
    
	AimOffsetYaw = HeroAnimProxy.CachedAimOffsetYaw;
	AimOffsetPitch = HeroAnimProxy.CachedAimOffsetPitch;

	GroundDistance = HeroAnimProxy.CachedGroundDistance;
}

