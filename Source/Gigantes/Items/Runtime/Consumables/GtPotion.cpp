#include "GtPotion.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

AGtPotion::AGtPotion()
{
	// 에디터 프리뷰용 기본값(데이터로 덮어쓰기 권장)
	HealAmount = 50;
	bDestroyOnUse = true;
	Charges = 1;          // 1회용 기본
	CooldownSeconds = 0.f;
}

void AGtPotion::InitFromData(const FGtItemData& InData)
{
	Super::InitFromData(InData);
	if (InData.HealAmount > 0)
	{
		HealAmount = InData.HealAmount;
	}
}

bool AGtPotion::ApplyEffect(AActor* User)
{
	if (!User)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Potion] User is null"));
		return false;
	}

	// 1) BP 훅이 true를 돌려주면 성공으로 처리
	if (BP_ApplyHeal(User, HealAmount))
	{
		return true;
	}

	// 2) C++ 기본 처리: (있다면) HealthComponent를 찾아서 Heal
	// 네 프로젝트에 UGtHealthComponent가 있다면 아래 주석 해제 + 맞게 수정
	/*
	if (UGtHealthComponent* Health = User->FindComponentByClass<UGtHealthComponent>())
	{
		const bool bHealed = Health->Heal(HealAmount);
		if (!bHealed)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Potion] Heal failed by HealthComponent"));
		}
		return bHealed;
	}
	*/

	// 3) 기본 경로 실패: BP로 구현하지 않았고 HealthComponent도 없음
	UE_LOG(LogTemp, Warning, TEXT("[Potion] No HealthComponent and BP hook not implemented"));
	return false;
}