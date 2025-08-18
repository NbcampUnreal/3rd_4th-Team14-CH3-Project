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
	
	AGtCharacterBase* OwningCharacter = Cast<AGtCharacterBase>(InAnimInstance->GetOwningActor());
    if (!IsValid(OwningCharacter)) return;

    UpdateMovementData(OwningCharacter);
    UpdateRotationData(OwningCharacter);
    UpdateStatusData(OwningCharacter);
}

void FGtBaseAnimInstanceProxy::UpdateMovementData(const AGtCharacterBase* Character)
{
	if (!Character) return;
    
	CachedVelocity = Character->GetVelocity();
    
	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (MovementComponent)
	{
		bCachedIsFalling = MovementComponent->IsFalling();
		bCachedIsMovingOnGround = MovementComponent->IsMovingOnGround();
		CachedAcceleration = MovementComponent->GetCurrentAcceleration();
	}
	else
	{
		bCachedIsFalling = false;
		bCachedIsMovingOnGround = false;
		CachedAcceleration = FVector::ZeroVector;
	}
}

void FGtBaseAnimInstanceProxy::UpdateRotationData(const AGtCharacterBase* Character)
{
	if (!Character) return;
    
	CachedActorRotation = Character->GetActorRotation();
}

void FGtBaseAnimInstanceProxy::UpdateStatusData(const AGtCharacterBase* Character)
{
	if (!Character) return;
    
	CachedStatusTags = Character->GetStatusTags();
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

	const auto& BaseAnimProxy = GetProxyOnAnyThread<FGtBaseAnimInstanceProxy>();

	StatusTags = BaseAnimProxy.CachedStatusTags;
    
	UpdateRotationValues(BaseAnimProxy);
	UpdateMovementStates(BaseAnimProxy);
	UpdateVelocityValues(BaseAnimProxy);
	UpdateAccelerationValues(BaseAnimProxy);
	
}

void UGtBaseAnimInstance::UpdateRotationValues(const FGtBaseAnimInstanceProxy& Proxy)
{
	ActorWorldRotation = Proxy.CachedActorRotation;
	YawDeltaLastFrame = ActorWorldRotation.Yaw - PrevActorWorldRotation.Yaw;
	PrevActorWorldRotation = ActorWorldRotation;
    
	LocomotionDirection = UKismetAnimationLibrary::CalculateDirection(
		Proxy.CachedVelocity, 
		Proxy.CachedActorRotation
	);
}

void UGtBaseAnimInstance::UpdateMovementStates(const FGtBaseAnimInstanceProxy& Proxy)
{
	bIsInAir = Proxy.bCachedIsFalling;
	bIsOnGround = Proxy.bCachedIsMovingOnGround;

	bIsJumping = bIsInAir && Proxy.CachedVelocity.Z > 0;
	bIsFalling = bIsInAir && Proxy.CachedVelocity.Z <= 0;

	GroundSpeed = Proxy.CachedVelocity.Size2D();
	FallSpeed = Proxy.CachedVelocity.Z;
}

void UGtBaseAnimInstance::UpdateVelocityValues(const FGtBaseAnimInstanceProxy& Proxy)
{
	const FVector WorldVelocity2D = Proxy.CachedVelocity * FVector(1.f, 1.f, 0.f);
	const FVector LocalVelocity2D = Proxy.CachedActorRotation.UnrotateVector(WorldVelocity2D);
    
	bHasVelocity = !UKismetMathLibrary::NearlyEqual_FloatFloat(
		UKismetMathLibrary::VSizeXYSquared(LocalVelocity2D), 
		0.0f
	);
}

void UGtBaseAnimInstance::UpdateAccelerationValues(const FGtBaseAnimInstanceProxy& Proxy)
{
	const FVector WorldAcceleration2D = Proxy.CachedAcceleration * FVector(1.f, 1.f, 0.f);
	const FVector LocalAcceleration2D = Proxy.CachedActorRotation.UnrotateVector(WorldAcceleration2D);
    
	bHasAcceleration = !UKismetMathLibrary::NearlyEqual_FloatFloat(
		UKismetMathLibrary::VSizeXYSquared(LocalAcceleration2D), 
		0.0f
	);
}

