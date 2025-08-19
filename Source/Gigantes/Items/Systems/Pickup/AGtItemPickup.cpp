#include "AGtItemPickup.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

#include "Items/Systems/Manager/GtItemManagerComponent.h"
#include "Items/Systems/Factory/GtItemFactory.h"

AGtItemPickup::AGtItemPickup()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);

    // 라인트레이스/인터랙션에 유리한 충돌 세팅
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Mesh->SetCollisionObjectType(ECC_WorldDynamic);
    Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // 인터랙션 트레이스용
    Mesh->SetGenerateOverlapEvents(true);

    PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
    PromptWidget->SetupAttachment(RootComponent);
    PromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
    PromptWidget->SetDrawAtDesiredSize(true);
    PromptWidget->SetVisibility(false);
}

void AGtItemPickup::BeginPlay()
{
	Super::BeginPlay();

	if (ItemId.IsEmpty() && !bUseOverrideData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pickup] %s has no ItemId and override disabled"), *GetName());
	}
}

void AGtItemPickup::BeginFocus_Implementation(AActor* /*Interactor*/)
{
	if (PromptWidget) PromptWidget->SetHiddenInGame(false);
}

void AGtItemPickup::EndFocus_Implementation(AActor* /*Interactor*/)
{
	if (PromptWidget) PromptWidget->SetHiddenInGame(true);
}

bool AGtItemPickup::CanInteract_Implementation(AActor* Interactor) const
{
	if (!IsValid(Interactor)) return false;

	// 거리 체크(제곱 거리로 미세 최적화)
	const float DistSq = FVector::DistSquared(Interactor->GetActorLocation(), GetActorLocation());
	if (DistSq > FMath::Square(InteractDistance)) return false;

	// 인벤토리 존재 여부
	return (Interactor->FindComponentByClass<UGtItemManagerComponent>() != nullptr);
}

void AGtItemPickup::Interact_Implementation(AActor* Interactor)
{
	if (GiveToInventory(Interactor))
	{
		Destroy();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pickup] Interact failed: %s"), *GetName());
	}
}

bool AGtItemPickup::GiveToInventory(AActor* Interactor)
{
	if (!IsValid(Interactor)) return false;

	UGtItemManagerComponent* Inv = Interactor->FindComponentByClass<UGtItemManagerComponent>();
	if (!Inv)
	{
		if (AActor* OwnerActor = Interactor->GetOwner())
		{
			Inv = OwnerActor->FindComponentByClass<UGtItemManagerComponent>();
		}
	}
	if (!Inv) return false;

	// 1) 최소 경로: ItemId만으로 추가 (JSON/Subsystem/Factory는 매니저 내부에서 처리)
	if (!ItemId.IsEmpty())
	{
		if (Inv->GiveItemById(ItemId))
		{
			return true;
		}
		UE_LOG(LogTemp, Warning, TEXT("[Pickup] GiveItemById failed: %s"), *ItemId);
	}

	// 2) (옵션) 오버라이드 데이터 사용 경로
#if 1
	// 네 매니저에 GiveItemFromData(const FGtItemData&)가 구현되어 있다면 사용
	if (bUseOverrideData)
	{
		if (Inv->GiveItemFromData(ItemDataOverride))
		{
			return true;
		}
		UE_LOG(LogTemp, Warning, TEXT("[Pickup] GiveItemFromData failed (override)"));
	}
#else
	// 매니저에 해당 함수가 없다면, 직접 스폰 후 인벤토리에 넣는 로직으로 바꿔도 됨:
	// if (bUseOverrideData)
	// {
	//     if (AGtItemBase* NewItem = UGtItemFactory::SpawnItem(GetWorld(), ItemDataOverride, FTransform::Identity))
	//     {
	//         // Inv->AddItem(NewItem); // ← 네 매니저에 맞춰 함수명 조정
	//         return true;
	//     }
	// }
#endif

	return false;
}