#include "GtLoadoutComponent.h"

#include "GameFramework/Character.h"
#include "Character/GtHeroCharacter.h"
#include "Equipments/GtEquippable.h"

#include "Items/Systems/DataSubSystem/GigantesItemDataSubsystem.h"
#include "Items/Runtime/Weapons/GtWeaponItem.h"
#include "Items/Systems/Factory/GtItemFactory.h"
#include "GtGameplayTags.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UGtLoadoutComponent::UGtLoadoutComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGtLoadoutComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<AGtHeroCharacter>(GetOwner());
    if (!OwnerCharacter.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("LoadoutComponent: Owner is not AGtHeroCharacter!"));
    }
    
    // 관리할 슬롯 초기화
    LoadoutSlots.Add({GtGameplayTags::Loadout_Slot_Weapon_Primary});
    LoadoutSlots.Add({GtGameplayTags::Loadout_Slot_Weapon_Secondary});
    LoadoutSlots.Add({GtGameplayTags::Loadout_Slot_Grenade});
    LoadoutSlots.Add({GtGameplayTags::Loadout_Slot_Consumable});

    // ❌ 테스트 무기 지연 장착 코드 삭제
    // GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::EquipTestWeapon);
}

void UGtLoadoutComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DeactivateCurrentWeaponSlot(); // 한 곳에서 정리
    Super::EndPlay(EndPlayReason);
}

void UGtLoadoutComponent::EquipItemToSlot(const FGtItemData& ItemData, const FGameplayTag& SlotTag)
{
    FGtLoadoutSlot* TargetSlot = FindSlotByTag(SlotTag);
    if (!TargetSlot)
    {
        UE_LOG(LogTemp, Warning, TEXT("Equip failed: Slot tag %s not found."), *SlotTag.ToString());
        return;
    }

    const bool bOverwritingActive = (ActiveWeaponSlotTag == SlotTag);

    if (bOverwritingActive)
    {
        DeactivateCurrentWeaponSlot();
    }
    
    TargetSlot->EquippedItemData = ItemData;
    TargetSlot->bHasItem = true;

    // 무기 슬롯이면 활성화 고려
    if (bOverwritingActive || !ActiveWeaponSlotTag.IsValid())
    {
        if (SlotTag.MatchesTag(GtGameplayTags::Loadout_Slot_Weapon))
        {
            ChangeActiveWeaponSlot(SlotTag);
        }
    }

    OnLoadoutSlotChanged.Broadcast(*TargetSlot);
}

void UGtLoadoutComponent::UnequipItemFromSlot(const FGameplayTag& SlotTag)
{
    FGtLoadoutSlot* TargetSlot = FindSlotByTag(SlotTag);
    if (!TargetSlot || !TargetSlot->bHasItem) return;

    if (ActiveWeaponSlotTag == SlotTag)
    {
        DeactivateCurrentWeaponSlot();
        ActiveWeaponSlotTag = FGameplayTag::EmptyTag;
    }

    TargetSlot->bHasItem = false;
    TargetSlot->EquippedItemData = FGtItemData();

    OnLoadoutSlotChanged.Broadcast(*TargetSlot);
}

void UGtLoadoutComponent::ChangeActiveWeaponSlot(const FGameplayTag& NewActiveSlotTag)
{
    const FGtLoadoutSlot* TargetSlot = FindSlotByTag(NewActiveSlotTag);
    if (!TargetSlot || !TargetSlot->bHasItem) return;

    if (!TargetSlot->SlotTag.MatchesTag(GtGameplayTags::Loadout_Slot_Weapon))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot activate non-weapon slot as active weapon."));
        return;
    }

    if (ActiveWeaponSlotTag == NewActiveSlotTag)
    {
        // 같은 단축키 → 해제
        DeactivateCurrentWeaponSlot();
        return;
    }

    if (ActiveWeaponSlotTag.IsValid())
    {
        DeactivateCurrentWeaponSlot();
    }

    ActivateNewWeaponSlot(NewActiveSlotTag);
}

void UGtLoadoutComponent::DeactivateCurrentWeaponSlot()
{
    if (!CurrentEquippedWeapon) return;

    if (FGtLoadoutSlot* CurrentSlot = FindSlotByTag(ActiveWeaponSlotTag))
    {
        // (선택) 무기 상태를 저장하려면 여기서 CurrentEquippedWeapon에서 필요한 상태를 읽어 CurrentSlot->EquippedItemData에 반영
        // 예: 탄창/내구도 등. API가 준비되면 적용.
        // const FGtItemData& RuntimeData = CurrentEquippedWeapon->GetItemDataChecked(); // 프로젝트에 따라 제공 여부 상이
        // CurrentSlot->EquippedItemData = RuntimeData; 
    }

    // 인터페이스 알림
    if (CurrentEquippedWeapon->Implements<UGtEquippable>())
    {
        IGtEquippable::Execute_OnUnequipped(CurrentEquippedWeapon);
    }

    // (임시) 맨손 애님 레이어 복구
    if (OwnerCharacter.IsValid())
    {
        if (UAnimInstance* Anim = OwnerCharacter->GetMesh()->GetAnimInstance())
        {
            if (TSubclassOf<UAnimInstance> UnarmedLayer = OwnerCharacter->GetUnarmedAnimLayer())
            {
                Anim->LinkAnimClassLayers(UnarmedLayer);
            }
        }
    }
    
    ActiveWeaponSlotTag = FGameplayTag::EmptyTag;

    CurrentEquippedWeapon->Destroy();
    CurrentEquippedWeapon = nullptr;

    OnEquipmentWeaponChanged.Broadcast(nullptr);
}

void UGtLoadoutComponent::ActivateNewWeaponSlot(const FGameplayTag& SlotTag)
{
    const FGtLoadoutSlot* TargetSlot = FindSlotByTag(SlotTag);
    if (!TargetSlot || !TargetSlot->bHasItem) return;

    if (!TargetSlot->SlotTag.MatchesTag(GtGameplayTags::Loadout_Slot_Weapon))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot activate non-weapon slot as active weapon."));
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World) return;

    // Items 파이프라인: Factory로 스폰 → 무기 캐스팅
    FTransform SpawnTf;
    AActor* Spawned = UGtItemFactory::SpawnItem(World, TargetSlot->EquippedItemData, SpawnTf);
    AGtWeaponItem* NewWeapon = Cast<AGtWeaponItem>(Spawned);
    if (!NewWeapon)
    {
        if (Spawned) { Spawned->Destroy(); }
        UE_LOG(LogTemp, Warning, TEXT("ActivateNewWeaponSlot: Spawned actor is not AGtWeaponItem (ItemId=%s)"),
               *TargetSlot->EquippedItemData.ItemId);
        return;
    }

    CurrentEquippedWeapon = NewWeapon;
    ActiveWeaponSlotTag = SlotTag;
    
    if (OwnerCharacter.IsValid())
    {
        CurrentEquippedWeapon->AttachToComponent(
            OwnerCharacter->GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale, // 정확히 소켓 정렬
            WeaponAttachSocketName
        );
    }

    // (임시) 맨손 레이어 비활성화
    if (OwnerCharacter.IsValid())
    {
        if (UAnimInstance* Anim = OwnerCharacter->GetMesh()->GetAnimInstance())
        {
            if (TSubclassOf<UAnimInstance> UnarmedLayer = OwnerCharacter->GetUnarmedAnimLayer())
            {
                Anim->UnlinkAnimClassLayers(UnarmedLayer);
            }
        }
    }
    
    if (CurrentEquippedWeapon->Implements<UGtEquippable>())
    {
        IGtEquippable::Execute_OnEquipped(CurrentEquippedWeapon, GetOwner());
    }

    OnEquipmentWeaponChanged.Broadcast(CurrentEquippedWeapon);
}

void UGtLoadoutComponent::UseItemInSlot(const FGameplayTag& SlotTag)
{
    const FGtLoadoutSlot* TargetSlot = FindSlotByTag(SlotTag);
    if (!TargetSlot || !TargetSlot->bHasItem) return;

    // 무기 슬롯은 "사용"이 아니라 "장착" 개념이라 여기서 사용 금지
    if (TargetSlot->SlotTag.MatchesTag(GtGameplayTags::Loadout_Slot_Weapon))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot use weapon slot as usable item."));
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    // 소모품/수류탄 등은 임시 스폰 → UseItem → 파괴
    FTransform Tf;
    AActor* Spawned = UGtItemFactory::SpawnItem(World, TargetSlot->EquippedItemData, Tf);
    if (AGtItemBase* Item = Cast<AGtItemBase>(Spawned))
    {
        Item->UseItem(OwnerCharacter.Get()); // 파생에서 true/false로 사용 결과 반환하도록 설계되어 있으면 활용 가능
        Spawned->Destroy();

        // (선택) 수량/쿨다운 등을 데이터에 반영하고 UI 갱신
        // FGtLoadoutSlot* SlotToUpdate = FindSlotByTag(SlotTag);
        // if (SlotToUpdate) { /* SlotToUpdate->EquippedItemData.Quantity--; ... */ OnLoadoutSlotChanged.Broadcast(*SlotToUpdate); }
    }
}

void UGtLoadoutComponent::PrimaryActionPressed()
{
    if (CurrentEquippedWeapon && CurrentEquippedWeapon->Implements<UGtEquippable>())
    {
        IGtEquippable::Execute_ExecutePrimaryActionPressed(CurrentEquippedWeapon);
    }
}

void UGtLoadoutComponent::PrimaryActionReleased()
{
    if (CurrentEquippedWeapon && CurrentEquippedWeapon->Implements<UGtEquippable>())
    {
        IGtEquippable::Execute_ExecutePrimaryActionReleased(CurrentEquippedWeapon);
    }
}

void UGtLoadoutComponent::SecondaryActionPressed()
{
    if (CurrentEquippedWeapon && CurrentEquippedWeapon->Implements<UGtEquippable>())
    {
        IGtEquippable::Execute_ExecuteSecondaryActionPressed(CurrentEquippedWeapon);
    }
}

void UGtLoadoutComponent::SecondaryActionReleased()
{
    if (CurrentEquippedWeapon && CurrentEquippedWeapon->Implements<UGtEquippable>())
    {
        IGtEquippable::Execute_ExecuteSecondaryActionReleased(CurrentEquippedWeapon);
    }
}

void UGtLoadoutComponent::ReloadAction()
{
    if (CurrentEquippedWeapon && CurrentEquippedWeapon->Implements<UGtEquippable>())
    {
        IGtEquippable::Execute_ExecuteReloadAction(CurrentEquippedWeapon);
    }
}

FGtLoadoutSlot* UGtLoadoutComponent::FindSlotByTag(const FGameplayTag& SlotTag)
{
    return LoadoutSlots.FindByPredicate([&](const FGtLoadoutSlot& Slot){ return Slot.SlotTag == SlotTag; });
}