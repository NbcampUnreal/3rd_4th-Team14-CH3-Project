#include "GtAttributeComponent.h"

UGtAttributeComponent::UGtAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGtAttributeComponent::SetAttributePrimary(const FGameplayTag& AttributePrimaryTag, const float NewValue)
{
	if (FAttributePrimaryData* AttributeData = Attributes.Find(AttributePrimaryTag))
	{
		const float OldValue = AttributeData->CurrentValue;

		// MinValue와 MaxValue 사이의 값으로 Clamp
		const float ClampedValue = FMath::Clamp(NewValue, AttributeData->MinValue, AttributeData->MaxValue);

		// 최종적으로 적용될 값이 이전 값과 동일하다면 리턴
		if (FMath::IsNearlyEqual(OldValue, ClampedValue))
		{
			return;
		}
		
		// 제한된 값으로 현재 값을 업데이트
		AttributeData->CurrentValue = ClampedValue;

		// 값이 변경되었음을 외부에 Broadcast
		BroadcastAttributePrimaryChange(AttributePrimaryTag, OldValue, AttributeData->CurrentValue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UGtAttributeComponent::SetAttributePrimary: Attribute with tag '%s' not found."), *AttributePrimaryTag.ToString());
	}
}

float UGtAttributeComponent::GetAttributePrimary(const FGameplayTag& AttributePrimaryTag) const
{
	if (const FAttributePrimaryData* Data = Attributes.Find(AttributePrimaryTag))
	{
		return Data->CurrentValue;
	}
	return 0.f;
}


void UGtAttributeComponent::BroadcastAttributePrimaryChange(const FGameplayTag& AttributePrimaryTag, const float OldValue, const float NewValue) const
{
	if (OnAttributePrimaryChanged.IsBound())
	{
		// 델리게이트를 호출하여 외부에 Tag와 관련된 값의 변경을 알림(UI 등)
		OnAttributePrimaryChanged.Broadcast(AttributePrimaryTag, OldValue, NewValue);
	}
}


