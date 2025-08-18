#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataAsset.h"
#include "Gigantes/Items/Data/ItemClassMapping.h"
#include "GtItemSystemSettings.generated.h"

/**
 * Project Settings > Game > Gigantes Item System
 * - 데이터 디렉터리와 매핑 DataAsset 지정
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Gigantes Item System"))
class GIGANTES_API UGtItemSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	/** JSON 아이템 데이터를 스캔할 Content 상대 경로 (예: /Game/Items/Data, /Game/Items/Weapons) */
	UPROPERTY(EditAnywhere, Config, Category="Data")
	TArray<FDirectoryPath> DataDirectories;

	/** GameplayTag -> 아이템 클래스 매핑 DataAsset */
	UPROPERTY(EditAnywhere, Config, Category="Data")
	TSoftObjectPtr<UItemClassMapping> ItemClassMapping;

	/** 로딩 화면에서 프리로드할 추가 소프트 오브젝트 경로(옵션) */
	UPROPERTY(EditAnywhere, Config, Category="Preload")
	TArray<FSoftObjectPath> ExtraPreloadAssets;
};
