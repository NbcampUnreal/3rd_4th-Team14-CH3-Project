#include "GigantesItemDataSubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Gigantes/Items/Systems/GtItemSystemSettings.h"
#include "Gigantes/Items/Runtime/Core/GtItemBase.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/Package.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/ReflectedTypeAccessors.h"
#include "Misc/PackageName.h"
#include "UObject/UnrealType.h"      // FIntProperty, FFloatProperty, FStrProperty, FStructProperty
#include "GameplayTagContainer.h"    // FGameplayTag
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"
#include "Components/MeshComponent.h"
#include "Kismet/GameplayStatics.h"

static const FName LogCat(TEXT("Gigantes.ItemData"));

const FGtItemData* UGigantesItemDataSubsystem::FindItemData(const FString& ItemId) const
{
	if (const FGtItemData* Found = ItemDataMap.Find(ItemId))
	{
		return Found; // FGtItemData* 반환
	}
	return nullptr;
}

void UGigantesItemDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	UE_LOG(LogTemp, Log, TEXT("[ItemDataSubsystem] Initialize"));
	ItemDataMap.Reset();
	TagToClassSoft.Reset();
	bPreloadFinished = false;
	PreloadHandle.Reset();

	if (!Cmd_TestSpawnItem)
	{
		Cmd_TestSpawnItem = IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("gt.TestSpawnItem"),
			TEXT("Spawn an item by Id using preloaded cache. Usage: gt.TestSpawnItem <ItemId>"),
			FConsoleCommandWithWorldAndArgsDelegate::CreateUObject(
				this, &UGigantesItemDataSubsystem::Test_SpawnItem
			)
		);
	}

	BuildClassMapFromSettings();
	ScanDataFromSettings();
	BeginAsyncPreload();
}

void UGigantesItemDataSubsystem::Deinitialize()
{
	if (Cmd_TestSpawnItem)
	{
		IConsoleManager::Get().UnregisterConsoleObject(Cmd_TestSpawnItem);
		Cmd_TestSpawnItem = nullptr;
	}
	
	ItemDataMap.Reset();
	TagToClassSoft.Reset();
	PreloadHandle.Reset();
	bPreloadFinished = false;
}

bool UGigantesItemDataSubsystem::FindItemData(const FString& ItemId, FGtItemData& OutData) const
{
	if (const FGtItemData* Found = ItemDataMap.Find(ItemId))
	{
		OutData = *Found;
		return true;
	}
	return false;
}

void UGigantesItemDataSubsystem::GetAllIds(TArray<FString>& OutIds) const
{
	ItemDataMap.GetKeys(OutIds);
}

UClass* UGigantesItemDataSubsystem::ResolveItemClass(const FGtItemData& Data) const
{
	// 프리로드 전제 → 이미 로드되어 있으면 Get(), 아니면 nullptr
	if (UClass* C = Data.ItemClass.Get())
		return C;

	if (const TSoftClassPtr<AGtItemBase>* Soft = TagToClassSoft.Find(Data.ItemTag))
		return Soft->Get();

	return nullptr;
}

UClass* UGigantesItemDataSubsystem::ResolveItemClassById(const FString& ItemId) const
{
	if (const FGtItemData* Found = ItemDataMap.Find(ItemId))
	{
		return ResolveItemClass(*Found);
	}
	return nullptr;
}

void UGigantesItemDataSubsystem::BuildClassMapFromSettings()
{
	const UGtItemSystemSettings* Settings = GetDefault<UGtItemSystemSettings>();
	TagToClassSoft.Reset();
	if (!Settings) return;

	UItemClassMapping* Mapping = Settings->ItemClassMapping.IsValid()
		? Settings->ItemClassMapping.Get()
		: Settings->ItemClassMapping.LoadSynchronous();

	if (!Mapping)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemDataSubsystem] ItemClassMapping is null"));
		return;
	}

	for (const FItemClassMappingRow& Row : Mapping->Mappings)
	{
		if (Row.ItemTag.IsValid() && Row.ItemClass.ToSoftObjectPath().IsValid())
		{
			TagToClassSoft.Add(Row.ItemTag, Row.ItemClass);
			UE_LOG(LogTemp, Log, TEXT("[ItemDataSubsystem] Map %s -> %s"),
				*Row.ItemTag.ToString(), *Row.ItemClass.ToSoftObjectPath().ToString());
		}
	}
}

void UGigantesItemDataSubsystem::ScanDataFromSettings()
{
	const UGtItemSystemSettings* Settings = GetDefault<UGtItemSystemSettings>();
	if (!Settings)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemDataSubsystem] No Settings"));
		return;
	}

	int32 TotalAdded = 0;
	for (const FDirectoryPath& Dir : Settings->DataDirectories)
	{
		if (Dir.Path.IsEmpty()) continue;
		ScanJsonDir(Dir.Path, ItemDataMap);
	}
	UE_LOG(LogTemp, Log, TEXT("[ItemDataSubsystem] Total Items: %d"), ItemDataMap.Num());
}

void UGigantesItemDataSubsystem::BeginAsyncPreload()
{
	TSet<FSoftObjectPath> ToLoad;

	// JSON에서 온 소프트 클래스 경로들
	for (const auto& KVP : ItemDataMap)
	{
		const FGtItemData& D = KVP.Value;

		// ItemClass 기준으로 수집
		if (!D.ItemClass.IsNull())
			ToLoad.Add(D.ItemClass.ToSoftObjectPath());

		// (선택) 태그 매핑도 같이 프리로드하고 싶으면 여기는 생략 가능
		// 아래에서 매핑을 일괄 추가하므로 실제로는 없어도 됨.
	}

	// 태그 → 클래스 매핑도 무조건 프리로드
	for (const auto& KVP : TagToClassSoft)
	{
		const TSoftClassPtr<AGtItemBase>& Soft = KVP.Value;
		if (!Soft.IsNull())
			ToLoad.Add(Soft.ToSoftObjectPath());
	}

	// 추가 프리로드(옵션)
	const UGtItemSystemSettings* Settings = GetDefault<UGtItemSystemSettings>();
	if (Settings)
	{
		for (const FSoftObjectPath& P : Settings->ExtraPreloadAssets)
		{
			if (P.IsValid()) ToLoad.Add(P);
		}
	}

	if (ToLoad.Num() == 0)
	{
		bPreloadFinished = true;
		OnReady.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("[ItemDataSubsystem] Nothing to preload"));
		return;
	}

	TArray<FSoftObjectPath> Paths = ToLoad.Array();
	UE_LOG(LogTemp, Log, TEXT("[ItemDataSubsystem] Preloading %d assets/classes"), Paths.Num());

	FStreamableManager& SM = UAssetManager::GetStreamableManager();
	PreloadHandle = SM.RequestAsyncLoad(
		Paths,
		FStreamableDelegate::CreateUObject(this, &UGigantesItemDataSubsystem::OnPreloadCompleted)
	);
}

void UGigantesItemDataSubsystem::OnPreloadCompleted()
{
	TagToClassHard.Reset();

	for (const auto& KVP : TagToClassSoft)
	{
		UClass* HardClass = nullptr;
		UObject* Loaded = KVP.Value.Get(); // 이미 로드된 상태

		if (UBlueprint* BP = Cast<UBlueprint>(Loaded))
		{
			HardClass = BP->GeneratedClass;
		}
		else
		{
			HardClass = Cast<UClass>(Loaded);
		}

		if (HardClass)
		{
			TagToClassHard.Add(KVP.Key, HardClass);
		}
	}

	
	bPreloadFinished = true;
	OnReady.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("[ItemDataSubsystem] Preload completed"));
}

// -------- 유틸: 디렉토리/파일 스캔 & JSON 파싱 --------
void UGigantesItemDataSubsystem::ScanJsonDir(const FString& LongPkgDir, TMap<FString, FGtItemData>& InOutMap)
{
	FString ContentAbsDir;
	if (!FPackageName::TryConvertLongPackageNameToFilename(LongPkgDir, ContentAbsDir))
	{
		// 사용자가 /Game/ 경로가 아닌 절대경로를 넣었다면 그대로 사용 시도
		ContentAbsDir = LongPkgDir;
	}
	if (!ContentAbsDir.EndsWith(TEXT("/"))) ContentAbsDir += TEXT("/");

	TArray<FString> Files;
	IFileManager::Get().FindFilesRecursive(Files, *ContentAbsDir, TEXT("*.json"), true, false);

	int32 AddedAll = 0;
	for (const FString& Abs : Files)
	{
		AddedAll += LoadOneJson(Abs, InOutMap);
	}
	UE_LOG(LogTemp, Log, TEXT("[ItemDataSubsystem] %s -> +%d"), *LongPkgDir, AddedAll);
}

int32 UGigantesItemDataSubsystem::LoadOneJson(const FString& AbsFilePath, TMap<FString, FGtItemData>& InOutMap)
{
	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *AbsFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("LoadOneJson: Cannot read %s"), *AbsFilePath);
		return 0;
	}

	// 루트를 Value로 파싱 (배열만 허용)
	TSharedPtr<FJsonValue> RootValue;
	{
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(Reader, RootValue) || !RootValue.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("LoadOneJson: Invalid JSON in %s"), *AbsFilePath);
			return 0;
		}
	}

	if (RootValue->Type != EJson::Array)
	{
		UE_LOG(LogTemp, Error, TEXT("LoadOneJson: Root must be a JSON array (file: %s)"), *AbsFilePath);
		return 0;
	}

	const TArray<TSharedPtr<FJsonValue>>& Arr = RootValue->AsArray();

	int32 Added = 0;
	for (const TSharedPtr<FJsonValue>& V : Arr)
	{
		const TSharedPtr<FJsonObject>* Obj = nullptr;
		if (!V.IsValid() || !V->TryGetObject(Obj) || !Obj || !Obj->IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("LoadOneJson: Non-object element skipped (%s)"), *AbsFilePath);
			continue;
		}

		FGtItemData Item;
		if (!FGtItemData::FromJson(Obj->ToSharedRef(), Item))
		{
			UE_LOG(LogTemp, Warning, TEXT("LoadOneJson: FromJson failed (%s)"), *AbsFilePath);
			continue;
		}

		if (Item.ItemId.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("LoadOneJson: Missing ItemId (%s)"), *AbsFilePath);
			continue;
		}

		if (InOutMap.Contains(Item.ItemId))
		{
			UE_LOG(LogTemp, Warning, TEXT("LoadOneJson: Duplicate ItemId='%s' (overwrite)"), *Item.ItemId);
		}

		InOutMap.Add(Item.ItemId, MoveTemp(Item));
		++Added;
	}

	UE_LOG(LogTemp, Log, TEXT("LoadOneJson: %s -> +%d"), *AbsFilePath, Added);
	return Added;
}

static const TCHAR* GtWorldTypeToText(EWorldType::Type T) {
    switch (T) {
    case EWorldType::Game: return TEXT("Game");
    case EWorldType::PIE: return TEXT("PIE");
    case EWorldType::Editor: return TEXT("Editor");
    case EWorldType::EditorPreview: return TEXT("EditorPreview");
    default: return TEXT("Other");
    }
}

void UGigantesItemDataSubsystem::Test_SpawnItem(const TArray<FString>& Args, UWorld* World)
{
    if (!World) { UE_LOG(LogTemp, Error, TEXT("[gt.TestSpawnItem] No world")); return; }
    if (Args.Num() < 1) { UE_LOG(LogTemp, Warning, TEXT("Usage: gt.TestSpawnItem <ItemId>")); return; }
    const FString ItemId = Args[0];

    if (!IsPreloadFinished()) {
        UE_LOG(LogTemp, Warning, TEXT("[gt.TestSpawnItem] Preload not finished yet"));
        return;
    }

    // 1) 월드 진단
    UE_LOG(LogTemp, Log, TEXT("[gt.TestSpawnItem] World=%s Type=%s NetMode=%d"),
        *GetNameSafe(World), GtWorldTypeToText(World->WorldType), (int32)World->GetNetMode());

    if (!(World->WorldType == EWorldType::PIE || World->WorldType == EWorldType::Game)) {
        UE_LOG(LogTemp, Warning, TEXT("[gt.TestSpawnItem] Not a Game/PIE world. Run during Play."));
    }

    // 2) 데이터/클래스 확보
    const FGtItemData* D = FindItemDataById(ItemId);
    if (!D) { UE_LOG(LogTemp, Error, TEXT("[gt.TestSpawnItem] No ItemData for %s"), *ItemId); return; }

    UClass* Cls = nullptr;

    // (1순위) JSON에 SoftClass가 있다면 사용
#if 1
    // FGtItemData에 필드가 'ItemClass' (TSoftClassPtr<AGtItemBase>) 라고 가정
    if (D->ItemClass.ToSoftObjectPath().IsValid()) {
        Cls = D->ItemClass.LoadSynchronous();
    }
#endif

    // (2순위) 태그→클래스 매핑
    if (!IsValid(Cls)) {
        Cls = GetHardClassByTag(D->ItemTag);
    }

    if (!IsValid(Cls)) { UE_LOG(LogTemp, Error, TEXT("[gt.TestSpawnItem] Resolve class failed")); return; }
    if (!Cls->IsChildOf(AGtItemBase::StaticClass())) {
        UE_LOG(LogTemp, Error, TEXT("[gt.TestSpawnItem] Class %s not derived from AGtItemBase"), *GetNameSafe(Cls));
        return;
    }
    if (Cls->HasAnyClassFlags(CLASS_Abstract)) {
        UE_LOG(LogTemp, Error, TEXT("[gt.TestSpawnItem] Class %s is ABSTRACT"), *GetNameSafe(Cls));
        return;
    }

    // 3) 스폰 위치: 카메라 앞 150cm
    FVector Loc = FVector::ZeroVector;
    FRotator Rot = FRotator::ZeroRotator;
    if (APlayerController* PC = World->GetFirstPlayerController()) {
        FVector CamLoc; FRotator CamRot;
        PC->GetPlayerViewPoint(CamLoc, CamRot);
        Loc = CamLoc + CamRot.Vector() * 150.f;
        Rot = CamRot;
    }
    const FTransform Xform(Rot, Loc);
    UE_LOG(LogTemp, Log, TEXT("[gt.TestSpawnItem] Spawn at %s Rot=%s Class=%s"),
        *Loc.ToString(), *Rot.Euler().ToString(), *GetNameSafe(Cls));

    // 4) 충돌 무시 + Defer 스폰
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Params.Name = MakeUniqueObjectName(World, Cls, FName(*FString::Printf(TEXT("GT_%s"), *ItemId)));

    AGtItemBase* Spawned = World->SpawnActorDeferred<AGtItemBase>(Cls, Xform, nullptr, nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

    if (!Spawned) {
        UE_LOG(LogTemp, Error, TEXT("[gt.TestSpawnItem] SpawnActorDeferred returned NULL"));
        return;
    }

    // 5) 데이터 적용
    Spawned->InitFromData(*D);

    // 6) 마무리 + 검증
    UGameplayStatics::FinishSpawningActor(Spawned, Xform);
	
	const bool bValid          = IsValid(Spawned);
	const bool bBeingDestroyed = Spawned->IsActorBeingDestroyed();
	const bool bDestroyFlags   = Spawned->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed);

	UE_LOG(LogTemp, Log, TEXT("[gt.TestSpawnItem] SPAWN OK Actor=%s Valid=%d BeingDestroyed=%d DestroyFlags=%d Level=%s World=%s"),
		*GetNameSafe(Spawned),
		bValid ? 1 : 0,
		bBeingDestroyed ? 1 : 0,
		bDestroyFlags ? 1 : 0,
		*GetNameSafe(Spawned->GetLevel()),
		*GetNameSafe(Spawned->GetWorld()));

    // 7) 시각 확인
    DrawDebugSphere(World, Loc, 20.f, 16, FColor::Green, false, 5.f, 0, 1.5f);
}

// void UGigantesItemDataSubsystem::Test_SpawnItem(const TArray<FString>& Args, UWorld* World)
// {
// 	if (!World)
//     {
//         UE_LOG(LogTemp, Error, TEXT("[gt.TestSpawnItem] No world"));
//         return;
//     }
//     if (Args.Num() < 1)
//     {
//         UE_LOG(LogTemp, Warning, TEXT("Usage: gt.TestSpawnItem <ItemId>"));
//         return;
//     }
//
//     const FString ItemId = Args[0];
//
//     if (!IsPreloadFinished())
//     {
//         UE_LOG(LogTemp, Warning, TEXT("[gt.TestSpawnItem] Preload not finished yet"));
//         return;
//     }
//
//     const FGtItemData* D = FindItemDataById(ItemId);
//     if (!D)
//     {
//         UE_LOG(LogTemp, Error, TEXT("[gt.TestSpawnItem] No ItemData for %s"), *ItemId);
//         return;
//     }
//
//     UClass* Cls = GetHardClassByTag(D->ItemTag);
//     if (!IsValid(Cls))
//     {
//         // (예외) JSON에 직접 들어있는 클래스가 이미 로드되어 있으면 사용
//         Cls = D->ItemClass.Get();
//     }
//     if (!IsValid(Cls))
//     {
//         UE_LOG(LogTemp, Error, TEXT("[gt.TestSpawnItem] No class for Tag=%s (ItemId=%s)"),
//             *D->ItemTag.ToString(), *ItemId);
//         return;
//     }
//
//     // 스폰
// 	APlayerController* PC = World->GetFirstPlayerController();
// 	FVector CamLoc = FVector::ZeroVector;
// 	FRotator CamRot = FRotator::ZeroRotator;
// 	if (PC) { PC->GetPlayerViewPoint(CamLoc, CamRot); }
//
// 	FVector SpawnLoc = CamLoc + CamRot.Vector() * 120.f; // 카메라 앞
// 	// 바닥으로 레이캐스트해서 파묻힘/공중 방지(선택)
// 	{
// 		FHitResult Hit;
// 		const FVector Start = SpawnLoc + FVector(0,0,50);
// 		const FVector End   = SpawnLoc - FVector(0,0,10000);
// 		FCollisionQueryParams Q(TEXT("gt.TestSpawnItemTrace"), false);
// 		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Q) && Hit.bBlockingHit)
// 		{
// 			SpawnLoc = Hit.Location + FVector(0,0,2); // 살짝 띄우기
// 			CamRot.Pitch = 0.f; // 월드 놓기라면 눈높이 각도 제거(선택)
// 		}
// 	}
//
// 	const FTransform SpawnTM(CamRot, SpawnLoc);
//
// 	// 항상 스폰 + 소유자/인스티게이터 지정
// 	FActorSpawnParameters Params;
// 	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
// 	Params.Owner      = PC ? PC->GetPawn() : nullptr;
// 	Params.Instigator = PC ? PC->GetPawn<APawn>() : nullptr;
//
// 	AActor* A = World->SpawnActor<AActor>(Cls, SpawnTM, Params);
// 	if (!A)
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("[gt.TestSpawnItem] Spawn failed for %s"), *ItemId);
// 		return;
// 	}
//
// 	// 가시성/충돌 강제 ON (BP 세팅이 '장착 전용'이라 숨김일 수 있어서)
// 	A->SetActorHiddenInGame(false);
// 	A->SetActorEnableCollision(true);
//
// 	TInlineComponentArray<UMeshComponent*> Meshes(A);
// 	for (UMeshComponent* MC : Meshes)
// 	{
// 		if (!MC) continue;
// 		MC->SetVisibility(true, true);
// 		MC->SetHiddenInGame(false);
// 		MC->SetRenderInMainPass(true);
// 		MC->SetOwnerNoSee(false);
// 		MC->SetOnlyOwnerSee(false);
// 		MC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
// 	}
//
// 	// 디버그 표시
// 	UE_LOG(LogTemp, Log, TEXT("[gt.TestSpawnItem] Spawned %s as %s @ %s"),
// 		*ItemId, *A->GetName(), *A->GetActorLocation().ToString());
//
// 	DrawDebugSphere(World, A->GetActorLocation(), 8.f, 12, FColor::Green, false, 3.f, 0, 1.f);
//
//     // ─────────────────────────────────────────────────────────────────────
//     // ① 화면/월드에 즉시 값 뿌리기 (초간단 시각 확인)
//     if (GEngine)
//     {
//         GEngine->AddOnScreenDebugMessage(
//             -1, 3.f, FColor::Green,
//             FString::Printf(TEXT("%s | Dmg=%d Ammo=%d Heal=%d FR=%.2f RT=%.2f"),
//                 *D->ItemName, D->Damage, D->MaxAmmo, D->HealAmount, D->FireRate, D->ReloadTime));
//     }
//     DrawDebugString(
//         World,
//         A->GetActorLocation() + FVector(0,0,120.f),
//         FString::Printf(TEXT("%s\nDmg=%d Ammo=%d Heal=%d\nFR=%.2f RT=%.2f"),
//             *D->ItemName, D->Damage, D->MaxAmmo, D->HealAmount, D->FireRate, D->ReloadTime),
//         A, FColor::Yellow, 5.f, true
//     );
//
// #if !UE_BUILD_SHIPPING
//     // ─────────────────────────────────────────────────────────────────────
//     // ② 액터에 같은 이름의 UPROPERTY가 있으면 값 꽂기 (없으면 자동 스킵)
//     auto TrySetInt = [&](const TCHAR* PropName, int32 Value)
//     {
//         if (FIntProperty* P = FindFProperty<FIntProperty>(A->GetClass(), PropName))
//             P->SetPropertyValue_InContainer(A, Value);
//     };
//     auto TrySetFloat = [&](const TCHAR* PropName, float Value)
//     {
//         if (FFloatProperty* P = FindFProperty<FFloatProperty>(A->GetClass(), PropName))
//             P->SetPropertyValue_InContainer(A, Value);
//     };
//     auto TrySetString = [&](const TCHAR* PropName, const FString& Value)
//     {
//         if (FStrProperty* P = FindFProperty<FStrProperty>(A->GetClass(), PropName))
//             P->SetPropertyValue_InContainer(A, Value);
//     };
//     auto TrySetTag = [&](const TCHAR* PropName, const FGameplayTag& Value)
//     {
//         if (FStructProperty* P = FindFProperty<FStructProperty>(A->GetClass(), PropName))
//         {
//             if (P->Struct == TBaseStructure<FGameplayTag>::Get())
//             {
//                 if (FGameplayTag* Ptr = P->ContainerPtrToValuePtr<FGameplayTag>(A))
//                     *Ptr = Value;
//             }
//         }
//     };
//
//     // 필요하면 아래 목록 중 필요한 것만 남겨도 됨
//     TrySetString(TEXT("ItemId"),          D->ItemId);
//     TrySetString(TEXT("ItemName"),        D->ItemName);
//     TrySetString(TEXT("ItemType"),        D->ItemType);
//     TrySetString(TEXT("SubType"),         D->SubType);
//     TrySetString(TEXT("Description"),     D->Description);
//
//     TrySetInt   (TEXT("Damage"),          D->Damage);
//     TrySetInt   (TEXT("MaxAmmo"),         D->MaxAmmo);
//     TrySetInt   (TEXT("AmmoInMagazine"),  D->AmmoInMagazine);
//
//     TrySetFloat (TEXT("FireRate"),        D->FireRate);
//     TrySetFloat (TEXT("ReloadTime"),      D->ReloadTime);
//
//     TrySetFloat (TEXT("ExplosionRadius"), D->ExplosionRadius);
//     TrySetFloat (TEXT("ExplosionDelay"),  D->ExplosionDelay);
//
//     TrySetInt   (TEXT("HealAmount"),      D->HealAmount);
//
//     TrySetTag   (TEXT("ItemTag"),         D->ItemTag);
//
//     // ─────────────────────────────────────────────────────────────────────
//     // ③ 꽂힌 값 즉석 검증(있을 때만). 같으면 Log, 다르면 Error
//     auto CheckInt = [&](const TCHAR* PropName, int32 Expected)
//     {
//         if (FIntProperty* P = FindFProperty<FIntProperty>(A->GetClass(), PropName))
//         {
//             const int32 Actual = P->GetPropertyValue_InContainer(A);
//             if (Actual == Expected)
//             {
//                 UE_LOG(LogTemp, Log, TEXT("  %-16s %d == %d"), PropName, Actual, Expected);
//             }
//             else
//             {
//                 UE_LOG(LogTemp, Error, TEXT("  %-16s %d != %d"), PropName, Actual, Expected);
//             }
//         }
//     };
//     auto CheckFloat = [&](const TCHAR* PropName, float Expected, float Tol=0.001f)
//     {
//         if (FFloatProperty* P = FindFProperty<FFloatProperty>(A->GetClass(), PropName))
//         {
//             const float Actual = P->GetPropertyValue_InContainer(A);
//             if (FMath::IsNearlyEqual(Actual, Expected, Tol))
//             {
//                 UE_LOG(LogTemp, Log, TEXT("  %-16s %.3f ~= %.3f"), PropName, Actual, Expected);
//             }
//             else
//             {
//                 UE_LOG(LogTemp, Error, TEXT("  %-16s %.3f != %.3f"), PropName, Actual, Expected);
//             }
//         }
//     };
// 	auto CheckString = [&](const TCHAR* PropName, const FString& Expected)
// 	{
// 		if (FStrProperty* P = FindFProperty<FStrProperty>(A->GetClass(), PropName))
// 		{
// 			const FString Actual = P->GetPropertyValue_InContainer(A);
// 			if (Actual == Expected)
// 			{
// 				UE_LOG(LogTemp, Log, TEXT("  %-16s \"%s\" == \"%s\""), PropName, *Actual, *Expected);
// 				
// 			}
// 			else
// 			{
// 				UE_LOG(LogTemp, Error, TEXT("  %-16s \"%s\" != \"%s\""), PropName, *Actual, *Expected);
// 				
// 			}
// 		}
// 		else
// 		{
// 			UE_LOG(LogTemp, Warning, TEXT("  %-16s <no property on %s> (JSON=\"%s\")"),
// 				PropName, *A->GetClass()->GetName(), *Expected);
// 		}
// 	};
// 	auto CheckTag = [&](const TCHAR* PropName, const FGameplayTag& Expected)
// 	{
// 		if (FStructProperty* P = FindFProperty<FStructProperty>(A->GetClass(), PropName))
// 		{
// 			if (P->Struct == TBaseStructure<FGameplayTag>::Get())
// 			{
// 				const FGameplayTag* Ptr = P->ContainerPtrToValuePtr<FGameplayTag>(A);
// 				const bool bOK = (Ptr && Ptr->MatchesTagExact(Expected));
// 				if (bOK)
// 				{
// 					UE_LOG(LogTemp, Log, TEXT("  %-16s %s == %s"), PropName, Ptr ? *Ptr->ToString() : TEXT("<null>"), *Expected.ToString());
// 					
// 				}
// 				else
// 				{
// 					
// 					UE_LOG(LogTemp, Error, TEXT("  %-16s %s != %s"), PropName, Ptr ? *Ptr->ToString() : TEXT("<null>"), *Expected.ToString());
// 				}
// 			}
// 			else
// 			{
// 				UE_LOG(LogTemp, Warning, TEXT("  %-16s exists but is not FGameplayTag on %s"), PropName, *A->GetClass()->GetName());
// 			}
// 		}
// 		else
// 		{
// 			UE_LOG(LogTemp, Warning, TEXT("  %-16s <no property on %s> (JSON=%s)"),
// 				PropName, *A->GetClass()->GetName(), *Expected.ToString());
// 		}
// 	};
//
//     CheckString(TEXT("ItemId"),          D->ItemId);
//     CheckString(TEXT("ItemName"),        D->ItemName);
//     CheckString(TEXT("ItemType"),        D->ItemType);
//     CheckString(TEXT("SubType"),         D->SubType);
//     CheckString(TEXT("Description"),     D->Description);
//
//     CheckInt   (TEXT("Damage"),          D->Damage);
//     CheckInt   (TEXT("MaxAmmo"),         D->MaxAmmo);
//     CheckInt   (TEXT("AmmoInMagazine"),  D->AmmoInMagazine);
//
//     CheckFloat (TEXT("FireRate"),        D->FireRate);
//     CheckFloat (TEXT("ReloadTime"),      D->ReloadTime);
//
//     CheckFloat (TEXT("ExplosionRadius"), D->ExplosionRadius);
//     CheckFloat (TEXT("ExplosionDelay"),  D->ExplosionDelay);
//
//     CheckInt   (TEXT("HealAmount"),      D->HealAmount);
//
//     CheckTag   (TEXT("ItemTag"),         D->ItemTag);
// #endif
// 	UE_LOG(LogTemp, Log, TEXT("  JSON.ItemId       = %s"), *D->ItemId);
// 	UE_LOG(LogTemp, Log, TEXT("  JSON.ItemName     = %s"), *D->ItemName);
// 	UE_LOG(LogTemp, Log, TEXT("  JSON.ItemType     = %s"), *D->ItemType);
// 	UE_LOG(LogTemp, Log, TEXT("  JSON.SubType      = %s"), *D->SubType);
// 	UE_LOG(LogTemp, Log, TEXT("  JSON.Description  = %s"), *D->Description);
// 	UE_LOG(LogTemp, Log, TEXT("  JSON.ItemTag      = %s"), *D->ItemTag.ToString());
// }

const FGtItemData* UGigantesItemDataSubsystem::FindItemDataById(const FString& Id) const
{
	return ItemDataMap.Find(Id);
}

UClass* UGigantesItemDataSubsystem::GetHardClassByTag(const FGameplayTag& Tag) const
{
	if (UClass* const* Found = TagToClassHard.Find(Tag))
	{
		return *Found; // UClass*
	}
	return nullptr;
}