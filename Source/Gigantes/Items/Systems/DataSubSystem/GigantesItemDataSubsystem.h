#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Gigantes/Items/Structs/FGtItemData.h"
#include "Gigantes/Items/Data/ItemClassMapping.h"
#include "Engine/StreamableManager.h"
#include "HAL/IConsoleManager.h" // 테스트
#include "GigantesItemDataSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemDataReady);

class AGtItemBase;
struct FGtItemData;
struct FStreamableHandle;

/**
 * 게임 시작 -> JSON을 선파싱/캐싱, 필요한 클래스는 비동기 프리로드.
 * 런타임에서는 파일 I/O 금지, 캐시만 참조.
 */
UCLASS()
class GIGANTES_API UGigantesItemDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	const FGtItemData* FindItemData(const FString& ItemId) const;
	
	// Subsystem 라이프사이클
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 데이터 조회
	UFUNCTION(BlueprintPure, Category="Gigantes|Items")
	bool FindItemData(const FString& ItemId, FGtItemData& OutData) const;

	UFUNCTION(BlueprintPure, Category="Gigantes|Items")
	const TMap<FString, FGtItemData>& GetAllItemData() const { return ItemDataMap; }

	UFUNCTION(BlueprintPure, Category="Gigantes|Items")
	bool IsReady() const { return bPreloadFinished; }

	UFUNCTION(BlueprintCallable, Category="Gigantes|Items")
	void GetAllIds(TArray<FString>& OutIds) const;

	// 클래스 해석(프리로드 후 즉시 사용 가능)
	UFUNCTION(BlueprintPure, Category="Gigantes|Items")
	UClass* ResolveItemClass(const FGtItemData& Data) const;

	UFUNCTION(BlueprintPure, Category="Gigantes|Items")
	UClass* ResolveItemClassById(const FString& ItemId) const;

	// 로딩 완료 알림
	UPROPERTY(BlueprintAssignable)
	FOnItemDataReady OnReady;

	// 스폰 아이템 데이터 테스트 ----------------------------------------------
	UFUNCTION(Exec)
	void Test_SpawnItem(const TArray<FString>& Args, UWorld* World);

	// 콘솔 커맨드 핸들
	IConsoleObject* Cmd_TestSpawnItem = nullptr;
	
	/** 캐시된 JSON에서 ItemId로 조회 */
	const FGtItemData* FindItemDataById(const FString& Id) const;

	/** 프리로딩 단계에서 고정된(하드) 클래스 매핑에서 GameplayTag로 조회 */
	UClass* GetHardClassByTag(const FGameplayTag& Tag) const;

	UPROPERTY()
	TMap<FGameplayTag, UClass*> TagToClassHard;
	// --------------------------------------------------------------------------------
	
	// UFUNCTION(BlueprintCallable, Category="ItemData")
	bool IsPreloadFinished() const { return bPreloadFinished; }

private:
	// 캐시
	UPROPERTY()
	TMap<FString, FGtItemData> ItemDataMap; // ItemId -> Data
	UPROPERTY()
	TMap<FGameplayTag, TSoftClassPtr<AGtItemBase>> TagToClassSoft; // Tag -> SoftClass
	UPROPERTY()
	bool bPreloadFinished = false;
	TSharedPtr<FStreamableHandle> PreloadHandle;

	// 내부 처리
	void ScanDataFromSettings();
	void BuildClassMapFromSettings();
	void BeginAsyncPreload();
	void OnPreloadCompleted();

	// 유틸
	static void ScanJsonDir(const FString& LongPkgDir, TMap<FString, FGtItemData>& InOutMap);
	static int32 LoadOneJson(const FString& AbsFilePath, TMap<FString, FGtItemData>& InOutMap);
};
