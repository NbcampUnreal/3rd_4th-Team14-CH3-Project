#include "GtEquipmentComponent.h"

#include "Gigantes/GtGameplayTags.h"
#include "Gigantes/Character/GtHeroCharacter.h"
#include "Gigantes/Equipments/GtEquippable.h"
#include "Gigantes/Items/Manager/GtItemFactory.h"
#include "Gigantes/Items/Manager/GtItemManagerComponent.h"
#include "Gigantes/Items/Weapons/GtWeaponItem.h"

UGtEquipmentComponent::UGtEquipmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGtEquipmentComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // ItemManager 찾기
    if (AActor* Owner = GetOwner())
    {
        ItemManagerComponent = Owner->FindComponentByClass<UGtItemManagerComponent>();
    }
}

void UGtEquipmentComponent::EquipWeapon(const FString& ItemId)
{
    if (!ItemManagerComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("ItemManagerComponent not found"));
        return;
    }
  
    // TODO : ItemManager에서 데이터 가져오기
    FGtItemData ItemData;
    // if (!ItemManagerComponent->GetItemDataById(ItemId, ItemData))
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("Item not found: %s"), *ItemId);
    //     return;
    // }

    // TODO : 현재 아이템 타입이 전역 태그에서 가져온 Item_Weapon 카테고리인지 체크
    // if (!ItemData.ItemTag.MatchesTag(FGameplayTag::RequestGameplayTag("Item.Weapon")))
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("%s is not a weapon"), *ItemId);
    //     return;
    // }
    
    // 아이템 생성
    AGtItemBase* CreatedItem = UGtItemFactory::CreateItem(ItemData, GetWorld());
    
    // AGtWeaponItem으로 캐스팅
    AGtWeaponItem* WeaponItem = Cast<AGtWeaponItem>(CreatedItem);
    if (!WeaponItem)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to cast to AGtWeaponItem"));
        if (CreatedItem)
        {
            CreatedItem->Destroy();
        }
        return;
    }
    
    // 기존 무기 해제
    if (CurrentWeapon)
    {
        UnequipWeapon();
    }
    
    CurrentWeapon = WeaponItem;
    
    // 소켓에 부착(추후 FGtItemData에서 소켓 이름, AnimLayer 등을 가지도록 함)
    if (USkeletalMeshComponent* OwnerMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
    {
        CurrentWeapon->AttachToComponent(OwnerMesh, 
            FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
            WeaponAttachSocketName);
    }
    
    // 장착 콜백
    IGtEquippable::Execute_OnEquipped(CurrentWeapon, GetOwner());
    
    // 델리게이트 호출
    OnEquipmentChanged.Broadcast(CurrentWeapon);
}

void UGtEquipmentComponent::UnequipWeapon()
{
    if (!CurrentWeapon)
        return;

    // AnimLayer 등의 정보를 활용해 UnLinkAnimLayer 등을 호출
    IGtEquippable::Execute_OnUnequipped(CurrentWeapon);
    
    // 액터 파괴
    CurrentWeapon->Destroy();
    CurrentWeapon = nullptr;
    
    // 델리게이트 호출
    OnEquipmentChanged.Broadcast(nullptr);
}

void UGtEquipmentComponent::PrimaryAction()
{
    if (!CurrentWeapon)
        return;
    
    // TODO : 캐릭터의 Dead 외에 추가적인 상태 체크 고려
    if (AGtHeroCharacter* Hero = Cast<AGtHeroCharacter>(GetOwner()))
    {
        // 죽은 상태면 무시
        if (Hero->HasStatusTag(GtGameplayTags::Status_Dead))
            return;
    }
    
    IGtEquippable::Execute_ExecutePrimaryAction(CurrentWeapon);
}

void UGtEquipmentComponent::SecondaryAction()
{
    if (!CurrentWeapon)
        return;
    
    IGtEquippable::Execute_ExecuteSecondaryAction(CurrentWeapon);
}

void UGtEquipmentComponent::ReloadAction()
{
    if (!CurrentWeapon)
        return;

    IGtEquippable::Execute_ExecuteReloadAction(CurrentWeapon);
}

