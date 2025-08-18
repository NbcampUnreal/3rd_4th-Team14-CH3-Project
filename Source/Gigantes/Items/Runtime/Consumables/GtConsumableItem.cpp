#include "GtConsumableItem.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

AGtConsumableItem::AGtConsumableItem()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGtConsumableItem::InitFromData(const FGtItemData& InData)
{
	Super::InitFromData(InData);
	// 소모품 공통 레벨에선 특별 매핑 없음.
	// (쿨타임/스택을 FGtItemData에 넣고 싶으면 구조체 확장해서 여기서 덮어쓰기)
}

bool AGtConsumableItem::UseItem(AActor* User)
{
	if (bOnCooldown)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Consumable] on cooldown"));
		return false;
	}

	if (Charges == 0 || Charges > 0) // 0=무제한, >0=남은 스택
	{
		// 효과 적용
		if (!ApplyEffect(User))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Consumable] ApplyEffect failed"));
			return false;
		}

		PlayUseEffects(User);

		// 스택 감소 처리
		if (Charges > 0)
		{
			Charges = FMath::Max(Charges - 1, 0);
		}

		// 쿨타임
		if (CooldownSeconds > 0.f)
		{
			bOnCooldown = true;
			GetWorldTimerManager().SetTimer(
				CooldownHandle, this, &AGtConsumableItem::EndCooldown, CooldownSeconds, false);
		}

		// 파괴 여부
		if (bDestroyOnUse && (Charges == 0))
		{
			Destroy();
		}

		return true;
	}

	return false;
}

bool AGtConsumableItem::ApplyEffect(AActor* User)
{
	// 파생에서 구현 예정(기본은 실패)
	return false;
}

void AGtConsumableItem::PlayUseEffects(AActor* User)
{
	if (UseFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this, UseFX,
			User ? User->GetActorLocation() : GetActorLocation(),
			User ? User->GetActorRotation() : GetActorRotation());
	}
	if (UseSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this, UseSFX,
			User ? User->GetActorLocation() : GetActorLocation());
	}
}

void AGtConsumableItem::EndCooldown()
{
	bOnCooldown = false;
}