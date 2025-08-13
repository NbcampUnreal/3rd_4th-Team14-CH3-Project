#include "GtEquipmentComponent.h"

#include "Gigantes/Equipments/GtEquippable.h"
#include "Gigantes/Items/Manager/GtItemFactory.h"
#include "Gigantes/Items/Manager/GtItemManagerComponent.h"
#include "Gigantes/Items/Weapons/GtWeaponItem.h"
#include "Gigantes/Character/Test/GtTestWeaponBase.h"


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

// TODO : Test 코드로써 삭제 필요
void UGtEquipmentComponent::EquipTestWeapon()
{
    // 기존 무기 해제
    if (CurrentWeapon)
    {
        UnequipWeapon();
    }
    
    // 블루프린트 클래스 체크
    if (!TestWeaponClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TestWeaponClass not set! Please set it in BP_HeroCharacter"));
        return;
    }
    
    // 테스트 무기 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner();
    
    CurrentWeapon = GetWorld()->SpawnActor<AGtWeaponItem>(
        TestWeaponClass,  // AGtWeaponItem 타입 보장
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams
    );
    
    if (!CurrentWeapon)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn test weapon"));
        return;
    }
    
    // 또는 AGtTestWeaponItem에 별도 초기화 함수 추가
    if (AGtTestWeaponBase* TestWeapon = Cast<AGtTestWeaponBase>(CurrentWeapon))
    {
        TestWeapon->InitializeTestWeapon();  // 커스텀 함수
    }
    
    // 소켓 부착
    if (USkeletalMeshComponent* OwnerMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
    {
        // 소켓 존재 확인
        if (OwnerMesh->DoesSocketExist(WeaponAttachSocketName))
        {
            CurrentWeapon->AttachToComponent(OwnerMesh, 
                FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
                WeaponAttachSocketName);
            
            UE_LOG(LogTemp, Warning, TEXT("Weapon attached to socket: %s"), 
                *WeaponAttachSocketName.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Socket not found: %s"), 
                *WeaponAttachSocketName.ToString());
            
            // 소켓이 없으면 본에 직접 부착
            CurrentWeapon->AttachToComponent(OwnerMesh, 
                FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
                FName("hand_r"));  // 본 이름
        }
    }
    
    // 장착 콜백
    if (CurrentWeapon->Implements<UGtEquippable>())
    {
        IGtEquippable::Execute_OnEquipped(CurrentWeapon, GetOwner());
    }
    
    OnEquipmentChanged.Broadcast(CurrentWeapon);
    
    UE_LOG(LogTemp, Warning, TEXT("Test weapon equipped: %s"), 
        *CurrentWeapon->GetClass()->GetName());
}
