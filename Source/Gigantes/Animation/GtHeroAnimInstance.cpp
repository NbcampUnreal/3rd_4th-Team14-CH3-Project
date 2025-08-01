// Fill out your copyright notice in the Description page of Project Settings.


#include "GtHeroAnimInstance.h"

#include "Gigantes/Character/GtHeroCharacter.h"

void UGtHeroAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningHeroCharacter = Cast<AGtHeroCharacter>(GetOwningActor());
}

void UGtHeroAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwningHeroCharacter.IsValid())
	{
		return;
	}

	const FRotator ControlRotation = OwningHeroCharacter->GetControlRotation();
	const FRotator ActorRotation = OwningHeroCharacter->GetActorRotation();

	AimOffsetPitch = ControlRotation.Pitch;

	FRotator DeltaRotation = ControlRotation - ActorRotation;
	DeltaRotation.Normalize(); 
	AimOffsetYaw = DeltaRotation.Yaw;
}
