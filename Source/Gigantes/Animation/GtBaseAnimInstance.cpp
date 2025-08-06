#include "GtBaseAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gigantes/Character/GtCharacterBase.h"
#include "Kismet/KismetMathLibrary.h"

FGtBaseAnimInstanceProxy::FGtBaseAnimInstanceProxy(UAnimInstance* Instance)
	: FAnimInstanceProxy(Instance)
{
}

void FGtBaseAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
	
	if (!InAnimInstance) return;
	
	if (AGtCharacterBase* OwningCharacter = Cast<AGtCharacterBase>(InAnimInstance->GetOwningActor()))
	{
		Velocity = OwningCharacter->GetVelocity();
		ActorRotation = OwningCharacter->GetActorRotation();
		CachedStatusTags = OwningCharacter->GetStatusTags();
		
        
		if (UCharacterMovementComponent* MovementComponent = OwningCharacter->GetCharacterMovement())
		{
			bIsFalling = MovementComponent->IsFalling();
			bIsMovingOnGround = MovementComponent->IsMovingOnGround();
			Acceleration = MovementComponent->GetCurrentAcceleration();
		}
	}
}

FAnimInstanceProxy* UGtBaseAnimInstance::CreateAnimInstanceProxy()
{
	return new FGtBaseAnimInstanceProxy(this);
}

void UGtBaseAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy)
{
	delete InProxy;
}

void UGtBaseAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	// 중요: 엔진 내부 프록시 사용
	const auto& BaseAnimProxy = GetProxyOnAnyThread<FGtBaseAnimInstanceProxy>();

	StatusTags = BaseAnimProxy.CachedStatusTags;
	GroundSpeed = BaseAnimProxy.Velocity.Size2D();
	LocomotionDirection = UKismetAnimationLibrary::CalculateDirection(BaseAnimProxy.Velocity, BaseAnimProxy.ActorRotation);
	bIsInAir = BaseAnimProxy.bIsFalling;
	bIsOnGround = BaseAnimProxy.bIsMovingOnGround;
	bIsJumping = bIsInAir && BaseAnimProxy.Velocity.Z > 0;
	bIsFalling = bIsInAir && BaseAnimProxy.Velocity.Z <= 0;
	FallSpeed = BaseAnimProxy.Velocity.Z;

	const FVector WorldVelocity2D = BaseAnimProxy.Velocity * FVector(1.f, 1.f, 0.f);
	const FVector LocalVelocity2D = BaseAnimProxy.ActorRotation.UnrotateVector(WorldVelocity2D);
	bHasVelocity = !UKismetMathLibrary::NearlyEqual_FloatFloat(UKismetMathLibrary::VSizeXYSquared(LocalVelocity2D), 0.f);
	
	const FVector WorldAcceleration2D = BaseAnimProxy.Acceleration * FVector(1.f, 1.f, 0.f);
	const FVector LocalAcceleration2D = BaseAnimProxy.ActorRotation.UnrotateVector(WorldAcceleration2D);
	bHasAcceleration = !UKismetMathLibrary::NearlyEqual_FloatFloat(UKismetMathLibrary::VSizeXYSquared(LocalAcceleration2D), 0.f);
	
}


