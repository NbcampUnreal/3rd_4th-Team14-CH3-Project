#include "GtItemManagerComponent.h"
#include "Gigantes/Items/Systems/Factory/GtItemFactory.h"
#include "Gigantes/Items/Systems/DataSubSystem/GigantesItemDataSubsystem.h"

UGtItemManagerComponent::UGtItemManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

TArray<FGtInventoryViewRow> UGtItemManagerComponent::GetInventoryView() const
{
    TArray<FGtInventoryViewRow> Out;
    Out.Reserve(Inventory.Num());

    for (int32 i = 0; i < Inventory.Num(); ++i)
    {
        AGtItemBase* Item = Inventory[i];
        if (!IsValid(Item)) continue;

        FGtInventoryViewRow Row;
        Row.Index = i;

        const FGtItemData& D = Item->GetItemData();

        const bool bKnown = D.ItemTag.IsValid() || !D.ItemId.IsEmpty();
        if (bKnown)
        {
            Row.DisplayName = FText::FromString(D.ItemId.IsEmpty()
                ? D.ItemTag.ToString()
                : D.ItemId);
            Row.Icon = nullptr; // TODO: D에 아이콘 있으면 연결
        }
        else
        {
            Row.DisplayName = FText::FromString(TEXT("Unknown Item"));
            Row.Icon = nullptr;
        }

        Row.Count = 1;
        Out.Add(MoveTemp(Row));
    }

    return Out;
}

bool UGtItemManagerComponent::PickupFromActor(AActor* PickupActor)
{
    if (!PickupActor) return false;

    if (AGtItemBase* WorldItem = Cast<AGtItemBase>(PickupActor))
    {
        WorldItem->SetOwner(GetOwner());
        WorldItem->SetActorEnableCollision(false);
        WorldItem->SetActorHiddenInGame(true);
        WorldItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        WorldItem->SetActorLocation(FVector(0,0,-100000.f));

        Inventory.Add(WorldItem);
        OnInventoryChanged.Broadcast();
        return true;
    }
    return false;
}

bool UGtItemManagerComponent::DropItem(int32 Index, const FTransform& Where)
{
    if (!Inventory.IsValidIndex(Index)) return false;
    AGtItemBase* Item = Inventory[Index];
    if (!IsValid(Item)) { Inventory.RemoveAt(Index); OnInventoryChanged.Broadcast(); return false; }

    Item->SetActorHiddenInGame(false);
    Item->SetActorEnableCollision(true);
    Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Item->SetActorTransform(Where);

    Inventory.RemoveAt(Index);
    OnInventoryChanged.Broadcast();
    return true;
}

bool UGtItemManagerComponent::GiveItemById(const FString& ItemId)
{
    UWorld* World = GetWorld();
    if (!World) return false;

    if (UGameInstance* GI = World->GetGameInstance())
    {
        if (auto* Sub = GI->GetSubsystem<UGigantesItemDataSubsystem>())
        {
            if (const FGtItemData* D = Sub->FindItemDataById(ItemId))
            {
                return GiveItemFromData(*D);
            }
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("GiveItemById failed: %s"), *ItemId);
    return false;
}

bool UGtItemManagerComponent::GiveItemFromData(const FGtItemData& Data)
{
    UWorld* World = GetWorld();
    if (!World) return false;

    // 1) 월드 스폰 표준 함수가 SpawnItem(World, Data, Transform)인 경우:
    if (AGtItemBase* NewItem = UGtItemFactory::SpawnItem(World, Data, FTransform::Identity))
    {
        Inventory.Add(NewItem);
        OnInventoryChanged.Broadcast();
        return true;
    }

    // 2) 인벤토리 전용 헬퍼가 있다면(있을 때만 사용):
    // if (AGtItemBase* NewItem = UGtItemFactory::CreateInventoryItem(World, Data))
    // {
    //     Inventory.Add(NewItem);
    //     OnInventoryChanged.Broadcast();
    //     return true;
    // }

    return false;
}

void UGtItemManagerComponent::UseItem(int32 Index)
{
    if (Inventory.IsValidIndex(Index) && IsValid(Inventory[Index]))
    {
        Inventory[Index]->UseItem(GetOwner());
        // 필요시 소비/파괴 처리 추가
        OnInventoryChanged.Broadcast();
    }
}