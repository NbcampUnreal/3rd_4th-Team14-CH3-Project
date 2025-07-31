// Fill out your copyright notice in the Description page of Project Settings.


#include "GtInputConfig.h"

UInputAction* UGtInputConfig::FindNativeInputActionByTag(const FGameplayTag& InInputTag) const
{
	for (const FGtInputActionConfig& NativeInputAction : NativeInputActions)
	{
		if (NativeInputAction.InputTag == InInputTag && NativeInputAction.InputAction)
		{
			return NativeInputAction.InputAction;
		}
	}
	return nullptr;
}
