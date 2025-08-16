#include "GtLoadoutComponent.h"

#include "GameFramework/Character.h"
#include "Gigantes/Equipments/GtEquippable.h"
#include "Gigantes/Items/Manager/GtItemFactory.h"
#include "Gigantes/Items/Manager/GtItemManagerComponent.h"
#include "Gigantes/Items/Weapons/GtWeaponItem.h"
#include "Gigantes/Character/Test/TestGtGameplayTags.h"
#include "Gigantes/GtGameplayTags.h"
#include "Gigantes/Character/GtHeroCharacter.h"
#include "Gigantes/Character/Test/GtTestWeaponBase.h"


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
    
    // 게임 시작 시 관리할 슬롯들을 미리 정의하고 초기화
    LoadoutSlots.Add({GtGameplayTags::Loadout_Slot_Weapon_Primary});
    LoadoutSlots.Add({GtGameplayTags::Loadout_Slot_Weapon_Secondary});
    LoadoutSlots.Add({GtGameplayTags::Loadout_Slot_Grenade});
    LoadoutSlots.Add({GtGameplayTags::Loadout_Slot_Consumable});

    // TODO : 테스트 코드로써 삭제 필요
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::EquipTestWeapon);
}

void UGtLoadoutComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if(CurrentEquippedWeapon)
    {
        CurrentEquippedWeapon->Destroy();
        CurrentEquippedWeapon = nullptr;
    }
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

    // 덮어쓰려는 슬롯이 현재 활성화된 무기 슬롯인지 미리 확인
    const bool bIsOverwritingActiveSlot = (ActiveWeaponSlotTag == SlotTag);

    // 만약 활성화된 무기 슬롯을 덮어쓰는 경우 이전 아이템을 비활성화
    if (bIsOverwritingActiveSlot)
    {
        DeactivateCurrentWeaponSlot();
    }
    
    // 슬롯 데이터를 새 아이템 정보로 갱신
    TargetSlot->EquippedItemData = ItemData;
    TargetSlot->bHasItem = true;

    // 덮어쓴 슬롯이 무기 슬롯이었거나 혹은 비어있는 무기 슬롯에 처음 무기를 장착하는 경우 무기 활성화 시도
    if (bIsOverwritingActiveSlot || !ActiveWeaponSlotTag.IsValid())
    {
        // 무기 타입일 경우에만 ChangeActiveSlot을 통해 활성화를 시도
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
    if (!TargetSlot || !TargetSlot->bHasItem)
    {
        return;
    }
    if (!TargetSlot->SlotTag.MatchesTag(GtGameplayTags::Loadout_Slot_Weapon))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot activate non-weapon slot as active weapon."));
        return;
    }

    // 현재 들고 있는 무기의 단축키를 누른 경우
    if (ActiveWeaponSlotTag == NewActiveSlotTag)
    {
        // 무기 장착 해제
        DeactivateCurrentWeaponSlot();
        return;
    }

    // 다른 무기로 교체하는 경우
    if (ActiveWeaponSlotTag.IsValid())
    {
        DeactivateCurrentWeaponSlot();
    }

    // 새로운 무기를 꺼내어 활성화
    ActivateNewWeaponSlot(NewActiveSlotTag);
}

void UGtLoadoutComponent::DeactivateCurrentWeaponSlot()
{
    if (!CurrentEquippedWeapon) return;

    FGtLoadoutSlot* CurrentSlot = FindSlotByTag(ActiveWeaponSlotTag);
    if (CurrentSlot)
    {
        // 핵심: 파괴 전, 액터의 현재 상태(남은 총알 등)를 데이터 슬롯에 저장
        // CurrentSlot->EquippedItemData = CurrentEquippedWeapon->GetItemData();
    }
    
    // TODO : 구조 변경할 수 있음
    if (CurrentEquippedWeapon->Implements<UGtEquippable>())
    {
        IGtEquippable::Execute_OnUnequipped(CurrentEquippedWeapon);
    }

    // 장착 해제시 장착 해제 애니메이션/애님 레이어 비활성화 필요.

    // TODO : 임시 코드
    if (OwnerCharacter.IsValid()) 
    {
        AGtHeroCharacter* HeroCharacter = OwnerCharacter.Get();
        UAnimInstance* AnimInstance = HeroCharacter->GetMesh()->GetAnimInstance();
        TSubclassOf<UAnimInstance> CharacterUnarmedLayer = HeroCharacter->GetUnarmedAnimLayer();

        if (AnimInstance && CharacterUnarmedLayer)
        {
            AnimInstance->LinkAnimClassLayers(CharacterUnarmedLayer);
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
    
    // =====================================================
    //                 TODO : 테스트를 위한 임시 코드
    // =====================================================
    // 이 코드 부분은 ItemFactory 수정 없이 테스트하기 위함이며 나중에 제거해야 함
    AGtWeaponItem* NewWeapon = nullptr; 
    if (TargetSlot->EquippedItemData.ItemTag == GtGameplayTags::Item_Weapon_TestRifle)
    {
        UE_LOG(LogTemp, Warning, TEXT("Bypassing ItemFactory for Test Weapon."));
        // LoadoutComponent에 설정된 TestWeaponClass를 직접 사용해 스폰
        if (TestWeaponClass)
        {
            NewWeapon = GetWorld()->SpawnActor<AGtWeaponItem>(TestWeaponClass);
            if (AGtTestWeaponBase* TestWeapon = Cast<AGtTestWeaponBase>(NewWeapon))
            {
                TestWeapon->InitializeTestWeapon();
            }
        }
    }
    else
    {
        // 기존 로직: 테스트 무기가 아닐 경우에만 ItemFactory를 사용합니다.
        NewWeapon = Cast<AGtWeaponItem>(UGtItemFactory::CreateItem(TargetSlot->EquippedItemData, GetWorld()));
    }
    // ==================================================
    //                  TODO : 테스트 코드 종료
    // ==================================================

    // 추후 다시 주석 활성화
    //AGtItemBase* NewItem = UGtItemFactory::CreateItem(TargetSlot->EquippedItemData, GetWorld());
    if (!NewWeapon) return;

    CurrentEquippedWeapon = NewWeapon;
    ActiveWeaponSlotTag = SlotTag;
    
    if (OwnerCharacter.IsValid())
    {
        CurrentEquippedWeapon->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WeaponAttachSocketName);
    }

    // TODO: 임시 코드
    if (OwnerCharacter.IsValid()) 
    {
        AGtHeroCharacter* HeroCharacter = OwnerCharacter.Get(); 

        UAnimInstance* AnimInstance = HeroCharacter->GetMesh()->GetAnimInstance();
        TSubclassOf<UAnimInstance> CharacterUnarmedLayer = HeroCharacter->GetUnarmedAnimLayer();
        if (AnimInstance && CharacterUnarmedLayer)
        {
            AnimInstance->UnlinkAnimClassLayers(CharacterUnarmedLayer);
        }
    }
    
    // TODO : 구조 변경할 수 있음
    if (CurrentEquippedWeapon->Implements<UGtEquippable>())
    {
        IGtEquippable::Execute_OnEquipped(CurrentEquippedWeapon, GetOwner());
    }
    
    // 장착시 장착 애니메이션/애님 레이어등 활성화 필요.

    OnEquipmentWeaponChanged.Broadcast(CurrentEquippedWeapon);
}

void UGtLoadoutComponent::UseItemInSlot(const FGameplayTag& SlotTag)
{
    const FGtLoadoutSlot* TargetSlot = FindSlotByTag(SlotTag);
    if (!TargetSlot || !TargetSlot->bHasItem) return;
    if (TargetSlot->SlotTag.MatchesTag(GtGameplayTags::Loadout_Slot_Weapon))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot use weapon slot as usable item."));
        return;
    }
    // 아이템 임시 생성 및 사용
    AGtItemBase* TempItem = UGtItemFactory::CreateItem(TargetSlot->EquippedItemData, GetWorld());
    if(TempItem)
    {
        // TODO: 향후 'IGtUsable' 같은 별도 인터페이스로 변경 고려
        // 현재는 IGtEquippable의 PrimaryAction을 사용의 의미로 호출.
        if(TempItem->Implements<UGtEquippable>())
        {
            //IGtEquippable::Execute_ExecutePrimaryAction(TempItem);
            
            // 사용 후 슬롯의 아이템 수량 감소
            // TargetSlot->EquippedItemData.Quantity--;
            //UE_LOG(LogTemp, Log, TEXT("Used item in slot %s. %d remaining."), *SlotTag.ToString(), TargetSlot->EquippedItemData.Quantity);

            // 수량이 0 이하면 슬롯에서 아이템을 완전히 제거
            // if (TargetSlot->EquippedItemData.Quantity <= 0)
            // {
            //     UE_LOG(LogTemp, Log, TEXT("Item depleted. Removing from slot %s."), *SlotTag.ToString());
            //     UnequipItemFromSlot(SlotTag);
            // }
        }
        // 임시 생성된 아이템은 자신의 로직에 따라 스스로 파괴해야 함
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

// TODO : Test 코드로써 삭제 필요
void UGtLoadoutComponent::EquipTestWeapon()
{
    if (!TestWeaponClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot equip test weapon: TestWeaponClass is not set."));
        return;
    }

    // 테스트용 아이템 데이터를 직접 생성
    FGtItemData TestData;
    TestData.ItemId = "test_rifle_01";
    TestData.ItemName = "Test Rifle (From Test Equip)";
    TestData.ItemTag = GtGameplayTags::Item_Weapon_TestRifle;
    TestData.Damage = 15;
    TestData.MaxAmmo = 30;
    TestData.AmmoInMagazine = 30;
    TestData.FireRate = 0.1f;
    TestData.ReloadTime = 2.0f;
    
    // 중요: ItemFactory가 이 데이터를 기반으로 올바른 클래스(TestWeaponClass)를 스폰할 수 있도록
    // ItemFactory의 ItemClassMap에 태그와 클래스를 등록 필요
    // 예: UGtItemFactory::InitItemClassMap() 에 아래 코드 추가
    // ItemClassMap.Add(FGameplayTag::RequestGameplayTag("Item.Weapon.TestRifle"), AGtTestWeaponBase::StaticClass());

    // 생성한 테스트 데이터를 사용하여 EquipItemToSlot 함수를 호출
    // 이렇게 하면 테스트 코드도 실제 장착 로직을 그대로 사용하게 되어 안정적
    UE_LOG(LogTemp, Warning, TEXT("Equipping test weapon '%s' to Primary slot."), *TestData.ItemName);
    EquipItemToSlot(TestData, GtGameplayTags::Loadout_Slot_Weapon_Primary);
}
