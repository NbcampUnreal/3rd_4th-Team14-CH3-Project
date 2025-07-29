#include "GtCharacterBase.h"
#include "Components/GtAttributeComponent.h"
#include "Gigantes/GtGameplayTags.h"

AGtCharacterBase::AGtCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AttributeComponent = CreateDefaultSubobject<UGtAttributeComponent>(TEXT("AttributeComponent"));
}

void AGtCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

float AGtCharacterBase::GetAttributePrimary(const FGameplayTag& AttributePrimaryTag) const
{
	if (AttributeComponent)
	{
		return AttributeComponent->GetAttributePrimary(AttributePrimaryTag);
	}
	return 0.f;
}

void AGtCharacterBase::SetAttributePrimary(const FGameplayTag& AttributePrimaryTag, float NewValue)
{
	if (AttributeComponent)
	{
		AttributeComponent->SetAttributePrimary(AttributePrimaryTag, NewValue);
	}
}

void AGtCharacterBase::AddToAttributePrimary(const FGameplayTag& AttributePrimaryTag, float Delta)
{
	if (AttributeComponent)
	{
		const float CurrentValue = GetAttributePrimary(AttributePrimaryTag);
		SetAttributePrimary(AttributePrimaryTag, CurrentValue + Delta);
	}
}

float AGtCharacterBase::GetHealth() const
{
	if (AttributeComponent)
	{
		return AttributeComponent->GetAttributePrimary(GtGameplayTags::Attribute_Primary_Health);
	}
	return 0.f;
}

void AGtCharacterBase::SetHealth(float NewValue)
{
	if (AttributeComponent)
	{
		AttributeComponent->SetAttributePrimary(GtGameplayTags::Attribute_Primary_Health, NewValue);
	}
}

void AGtCharacterBase::AddHealth(float Delta)
{
	if (AttributeComponent)
	{
		const float CurrentValue = GetHealth();
		SetHealth(CurrentValue + Delta);
	}
}

bool AGtCharacterBase::HasStatusTagExact(const FGameplayTag& StatusTag) const
{
	return StatusTags.HasTagExact(StatusTag);
}

void AGtCharacterBase::AddStatusTag(const FGameplayTag& StatusTag)
{
	if (StatusTags.HasTagExact(StatusTag))
		return;

	StatusTags.AddTag(StatusTag);
	OnStatusTagChanged.Broadcast(StatusTag, /*bAdded=*/true);
}

void AGtCharacterBase::RemoveStatusTag(const FGameplayTag& StatusTag)
{
	if (!StatusTags.HasTagExact(StatusTag))
		return;
	
	StatusTags.RemoveTag(StatusTag);
	OnStatusTagChanged.Broadcast(StatusTag, /*bAdded=*/false);
}

float AGtCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}


