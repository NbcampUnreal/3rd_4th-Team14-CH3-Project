// Fill out your copyright notice in the Description page of Project Settings.


#include "GtBaseAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gigantes/Character/GtCharacterBase.h"

void UGtBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (AActor* OwningActor = GetOwningActor())
	{
		OwningCharacter = Cast<AGtCharacterBase>(OwningActor);
		if (OwningCharacter.IsValid())
		{
			MovementComponent = OwningCharacter->GetCharacterMovement();
		}
	}
}

void UGtBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter.IsValid() ||!MovementComponent.IsValid())
	{
		return;
	}

	GroundSpeed = OwningCharacter->GetVelocity().Size2D();
	LocomotionDirection = UKismetAnimationLibrary::CalculateDirection(OwningCharacter->GetVelocity(), OwningCharacter->GetActorRotation());
	bIsInAir = MovementComponent->IsFalling();

	StatusTags = OwningCharacter->GetStatusTags();
}

