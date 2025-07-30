#include "GtCharacterBase.h"
#include "Components/GtAttributeComponent.h"

AGtCharacterBase::AGtCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AttributeComponent = CreateDefaultSubobject<UGtAttributeComponent>(TEXT("AttributeComponent"));
}

void AGtCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	AttributeChangedHandlers.Add(GtGameplayTags::Attribute_Primary_Health, FAttributeChangedHandler::CreateUObject(this, &AGtCharacterBase::HandleHealthChanged));
	if (AttributeComponent)
	{
		AttributeComponent->OnAttributePrimaryChanged.AddDynamic(this, &AGtCharacterBase::OnAttributePrimaryChanged);
	}
}

void AGtCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AttributeChangedHandlers.Empty();
	if (AttributeComponent)
	{
		AttributeComponent->OnAttributePrimaryChanged.RemoveDynamic(this, &ThisClass::OnAttributePrimaryChanged);
	}
	
	Super::EndPlay(EndPlayReason);
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

void AGtCharacterBase::OnAttributePrimaryChanged(const FGameplayTag& AttributePrimaryTag, float OldValue, float NewValue)
{
	if (const FAttributeChangedHandler* AttributeChangedHandler = AttributeChangedHandlers.Find(AttributePrimaryTag))
	{
		AttributeChangedHandler->ExecuteIfBound(OldValue, NewValue);
	}
}

void AGtCharacterBase::HandleHealthChanged(float OldValue, float NewValue)
{
	FAttributePrimaryData Data;
	float Min = 0.f;
	if (AttributeComponent && AttributeComponent->GetAttributePrimaryData(GtGameplayTags::Attribute_Primary_Health, Data))
	{
		Min = Data.MinValue;
	}

	// Old > Min && New <= Min 일 때만 최초 사망
	if (OldValue > Min && NewValue <= Min)
	{
		Die();
	}
}

void AGtCharacterBase::Die()
{
	if (HasStatusTagExact(GtGameplayTags::Status_Dead))
	{
		UE_LOG(LogTemp, Warning, TEXT("Character is already dead."));
		return;
	}

	AddStatusTag(GtGameplayTags::Status_Dead);
	
	UE_LOG(LogTemp, Warning, TEXT("Character is dead."));
}

float AGtCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}


