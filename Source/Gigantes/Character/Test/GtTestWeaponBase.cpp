// Fill out your copyright notice in the Description page of Project Settings.


#include "GtTestWeaponBase.h"

#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Gigantes/Gameplay/Damage/GtDamageable.h"
#include "Gigantes/Physics/GtCollisionChannels.h"


AGtTestWeaponBase::AGtTestWeaponBase()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentAmmo = TestMaxAmmo;

    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(Sphere);
}

void AGtTestWeaponBase::InitializeTestWeapon()
{
    // 테스트 데이터 직접 설정
    ItemData.ItemId = "test_rifle_01";
    ItemData.ItemName = "Test Rifle";
    ItemData.ItemTag = FGameplayTag::RequestGameplayTag("Item.Weapon.TestRifle");
    ItemData.Damage = 30;
    ItemData.MaxAmmo = 30;
    ItemData.AmmoInMagazine = 30;
    ItemData.FireRate = 0.1f;
    ItemData.ReloadTime = 2.0f;
    
    // AGtWeaponItem의 멤버 변수들도 설정
    Damage = ItemData.Damage;
    MaxAmmo = ItemData.MaxAmmo;
    AmmoInMagazine = ItemData.AmmoInMagazine;
    FireRate = ItemData.FireRate;
    ReloadRate = ItemData.ReloadTime;
}

void AGtTestWeaponBase::OnEquipped_Implementation(AActor* NewOwner)
{
    WeaponOwner = NewOwner;
    CurrentAmmo = TestMaxAmmo;
    bCanFire = true;
    
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(NewOwner))
    {
        // 애님 레이어가 유효한지 확인하고 연결합니다.
        if (ArmedAnimLayer)
        {
            OwnerCharacter->GetMesh()->GetAnimInstance()->LinkAnimClassLayers(ArmedAnimLayer);
            UE_LOG(LogTemp, Log, TEXT("Armed Anim Layer Linked."));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[TestWeapon] Equipped to %s"), 
        NewOwner ? *NewOwner->GetName() : TEXT("Unknown"));
}

void AGtTestWeaponBase::OnUnequipped_Implementation()
{
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(WeaponOwner))
    {
        // 애님 레이어가 유효한지 확인하고 연결을 해제합니다.
        if (ArmedAnimLayer)
        {
            OwnerCharacter->GetMesh()->GetAnimInstance()->UnlinkAnimClassLayers(ArmedAnimLayer);
            UE_LOG(LogTemp, Log, TEXT("Armed Anim Layer Unlinked."));
        }
    }

    WeaponOwner = nullptr;
    GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
    
    UE_LOG(LogTemp, Warning, TEXT("[TestWeapon] Unequipped"));
}

void AGtTestWeaponBase::ExecutePrimaryAction_Implementation()
{
    TestFire();
}

void AGtTestWeaponBase::ExecuteSecondaryAction_Implementation()
{
    // 조준 테스트
    UE_LOG(LogTemp, Warning, TEXT("[TestWeapon] Aiming"));
}

void AGtTestWeaponBase::ExecuteReloadAction_Implementation()
{
    TestReload();
}

void AGtTestWeaponBase::TestFire()
{
    if (!bCanFire || CurrentAmmo <= 0 || !WeaponOwner)
    {
        UE_LOG(LogTemp, Warning, TEXT("[TestWeapon] Cannot fire"));
        return;
    }

    // TODO : 멤버 변수로 캐싱 대신에 GetOwner 사용 고려
    APawn* OwnerPawn = Cast<APawn>(WeaponOwner);
    if (!OwnerPawn)
        return;
    
    CurrentAmmo--;
    bCanFire = false;
    
    // 크로스헤어 LineTrace 결과를 그대로 사용
    FHitResult HitResult;
    bool bHit = GetCrosshairHitResult(HitResult);
    
    // 디버그 표시
    if (bShowDebugLine)
    {
        // 시작점에서 히트 지점까지 라인
        FVector TraceStart = HitResult.TraceStart;
        FVector TraceEnd = bHit ? HitResult.Location : HitResult.TraceEnd;
        
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, 
            bHit ? FColor::Green : FColor::Red, 
            false, DebugLineDuration, 0, 2.0f);
        
        if (bHit)
        {
            // 히트 포인트
            DrawDebugSphere(GetWorld(), HitResult.Location, 
                10.0f, 12, FColor::Yellow, false, DebugLineDuration);
        }
    }
    
    // 데미지 처리
    if (bHit && HitResult.GetActor())
    {
        if (HitResult.GetActor()->Implements<UGtDamageable>())
        {
            FGtDamageInfo DamageInfo;
            DamageInfo.BaseDamage = TestDamage;
            DamageInfo.DamageCauser = this;
            DamageInfo.Instigator = OwnerPawn->GetController();
            DamageInfo.HitResultInfo = HitResult;
            
            FGtDamageResult DamageResult;
            IGtDamageable::Execute_ApplyDamage(HitResult.GetActor(), DamageInfo, DamageResult);
            
            UE_LOG(LogTemp, Warning, TEXT("[TestWeapon] Hit %s for %.1f damage"), 
                *HitResult.GetActor()->GetName(), DamageResult.FinalDamage);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[TestWeapon] Fire! Ammo: %d/%d"), CurrentAmmo, TestMaxAmmo);
    
    // 발사 쿨다운
    GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, [this]()
    {
        bCanFire = true;
    }, TestFireRate, false);
}

void AGtTestWeaponBase::TestReload()
{
    CurrentAmmo = TestMaxAmmo;
    UE_LOG(LogTemp, Warning, TEXT("[TestWeapon] Reloaded! Ammo: %d/%d"), CurrentAmmo, TestMaxAmmo);
}

bool AGtTestWeaponBase::GetCrosshairHitResult(FHitResult& OutHitResult) const
{
    APawn* OwnerPawn = Cast<APawn>(WeaponOwner);
    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
    
    // 카메라 정보
    FVector CamLoc;
    FRotator CamRot;
    PC->GetPlayerViewPoint(CamLoc, CamRot);
    FVector AimDir = CamRot.Vector().GetSafeNormal();
    
    // 초점 거리
    const double FocalDistance = 1024.0f;
    FVector FocalLoc = CamLoc + (AimDir * FocalDistance);
    
    // 폰 위치를 조준선에 투영
    const FVector PawnLoc = OwnerPawn->GetActorLocation();
    FVector ProjectedStartLoc = FocalLoc + (((PawnLoc - FocalLoc) | AimDir) * AimDir);
    
    // 투영된 위치에서 조준 방향으로 LineTrace (데미지 판정용)
    FVector Start = ProjectedStartLoc;
    FVector End = Start + AimDir * 10000.0f;
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(WeaponOwner);
    QueryParams.AddIgnoredActor(this);
    //QueryParams.bTraceComplex = true;  // 정확한 히트 위치
    //QueryParams.bReturnPhysicalMaterial = true;  // 물리 머티리얼 (헤드샷 등)
    
    // ECC_Visibility로 모든 것 체크 (벽, 적 등)
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        OutHitResult,
        Start, 
        End, 
        Gt_TraceChannel_Weapon_Capsule, 
        QueryParams
    );
    
    return bHit;
}

