#include "GtCharacterBase.h"
#include "Components/GtAttributeComponent.h"
#include "Gigantes/GtGameplayTags.h"
#include "Gigantes/Gameplay/Damage/Components/GtDamageReceiverComponent.h"


AGtCharacterBase::AGtCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AttributeComponent = CreateDefaultSubobject<UGtAttributeComponent>(TEXT("AttributeComponent"));
	DamageReceiverComponent = CreateDefaultSubobject<UGtDamageReceiverComponent>(TEXT("DamageReceiverComponent"));
}

void AGtCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	AttributeChangedHandlers.Add(GtGameplayTags::Attribute_Primary_Health, FAttributeChangedHandler::CreateUObject(this, &AGtCharacterBase::HandleHealthChanged));
	if (AttributeComponent)
	{
		AttributeComponent->OnAttributePrimaryChanged.AddDynamic(this, &AGtCharacterBase::OnAttributePrimaryChanged);
	}

	if (DamageReceiverComponent)
	{
		DamageReceiverComponent->OnDamageProcessed.AddDynamic(this, &AGtCharacterBase::HandleDamageResult);
	}
}

bool AGtCharacterBase::ApplyDamage_Implementation(const FGtDamageInfo& DamageInfo, FGtDamageResult& OutDamageResult)
{
	if (HasStatusTagExact(GtGameplayTags::Status_Dead))
	{
		UE_LOG(LogTemp, Verbose, TEXT("[%s] Already dead. Ignore damage %.2f"), *GetName(), DamageInfo.BaseDamage);
		OutDamageResult.FinalDamage = 0.f;
		return false;
	}

	if (DamageReceiverComponent)
	{
		const bool bDamaged = DamageReceiverComponent->Execute_ApplyDamage(DamageReceiverComponent, DamageInfo, OutDamageResult);
		return bDamaged;
	}

	UE_LOG(LogTemp, Warning, TEXT("[%s] No DamageReceiverComponent."), *GetName());
	return false;
}


void AGtCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AttributeChangedHandlers.Empty();
	if (AttributeComponent)
	{
		AttributeComponent->OnAttributePrimaryChanged.RemoveDynamic(this, &ThisClass::OnAttributePrimaryChanged);
	}
	if (DamageReceiverComponent)
	{
		DamageReceiverComponent->OnDamageProcessed.RemoveDynamic(this, &ThisClass::HandleDamageResult);
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
	float MinHealth = 0.f;
	if (AttributeComponent && AttributeComponent->GetAttributePrimaryData(GtGameplayTags::Attribute_Primary_Health, Data))
	{
		MinHealth = Data.MinValue;
	}

	// OldValue > MinHealth && NewValue <= MinHealth 일 때만 최초 사망
	if (OldValue > MinHealth && NewValue <= MinHealth)
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

void AGtCharacterBase::HandleDamageResult(const FGtDamageResult& DamageResult)
{
	UE_LOG(LogTemp, Log, TEXT("[GtCharacterBase] Handling damage result. Final Damage: %.2f"), DamageResult.FinalDamage);
    
	// AttributeComponent에 체력 감소 요청
	if (DamageResult.FinalDamage > 0.f)
	{
		// 체력이 0이 될 경우 AttributeComponent의 델리게이트로 인해 HandleHealthChanged에서 Die 호출
		AddHealth(-DamageResult.FinalDamage);
	}
	
}

