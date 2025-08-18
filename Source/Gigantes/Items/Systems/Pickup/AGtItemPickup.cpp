#include "AGtItemPickup.h"

#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "../Manager/GtItemManagerComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AGtItemPickup::AGtItemPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetSimulatePhysics(false);

	PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
	PromptWidget->SetupAttachment(RootComponent);
	PromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
	PromptWidget->SetDrawAtDesiredSize(true);
	PromptWidget->SetVisibility(false);
}

void AGtItemPickup::BeginFocus_Implementation(AActor* Interactor)
{
	if (PromptWidget) PromptWidget->SetVisibility(true);
	// 필요하면 머티리얼 하이라이트, 나이아가라 등
}

void AGtItemPickup::EndFocus_Implementation(AActor* Interactor)
{
	if (PromptWidget) PromptWidget->SetVisibility(false);
}

bool AGtItemPickup::CanInteract_Implementation(AActor* Interactor) const
{
	// 간단히 거리로 제한 (원하면 더 정교하게)
	if (!Interactor) return false;
	const float Dist = FVector::Dist(Interactor->GetActorLocation(), GetActorLocation());
	return Dist <= 250.f;
}

void AGtItemPickup::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor) return;

	if (UGtItemManagerComponent* Inv = Interactor->FindComponentByClass<UGtItemManagerComponent>())
	{
		// 아이템 ID가 있으면 ID 기준, 없으면 현재 데이터 그대로 스폰
		if (!ItemData.ItemId.IsEmpty())
		{
			if (Inv->GiveItemById(ItemData.ItemId))
			{
				Destroy();
				return;
			}
		}
		else
		{
			// 데이터 직접 스폰이 필요하면 별도 API를 제공하거나 ItemId를 강제
			UE_LOG(LogTemp, Warning, TEXT("[Pickup] ItemId is empty; set ItemId for pickup items."));
		}
	}
}