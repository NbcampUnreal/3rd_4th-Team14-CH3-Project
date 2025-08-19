#include "FGtItemData.h"

#include "Dom/JsonObject.h"
#include "UObject/SoftObjectPath.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "Gigantes/Items/Runtime/Core/GtItemBase.h"

bool FGtItemData::FromJson(const TSharedPtr<FJsonObject>& Obj, FGtItemData& Out)
{
	if (!Obj.IsValid()) return false;

	Out.ItemId      = Obj->GetStringField(TEXT("ItemId"));
	Out.ItemName    = Obj->GetStringField(TEXT("ItemName"));
	Out.ItemType    = Obj->GetStringField(TEXT("ItemType"));
	Out.SubType     = Obj->GetStringField(TEXT("SubType"));
	Out.Description = Obj->GetStringField(TEXT("Description"));

	FString TagStr;
	if (Obj->TryGetStringField(TEXT("ItemTag"), TagStr))
	{
		// Config( DefaultGameplayTags.ini )에 등록된 문자열일 것
		Out.ItemTag = FGameplayTag::RequestGameplayTag(FName(*TagStr), /*ErrorIfNotFound=*/false);
	}

	FString ClassPath;
	if (Obj->TryGetStringField(TEXT("ClassPath"), ClassPath))
	{
		Out.ItemClass = TSoftClassPtr<AGtItemBase>(FSoftObjectPath(ClassPath));
	}

	// 타입별 선택 필드
	Obj->TryGetNumberField(TEXT("Damage"),            Out.Damage);
	Obj->TryGetNumberField(TEXT("MaxAmmo"),           Out.MaxAmmo);
	Obj->TryGetNumberField(TEXT("AmmoInMagazine"),    Out.AmmoInMagazine);
	Obj->TryGetNumberField(TEXT("FireRate"),          Out.FireRate);
	Obj->TryGetNumberField(TEXT("ReloadTime"),        Out.ReloadTime);
	Obj->TryGetNumberField(TEXT("ExplosionRadius"),   Out.ExplosionRadius);
	Obj->TryGetNumberField(TEXT("ExplosionDelay"),    Out.ExplosionDelay);
	Obj->TryGetNumberField(TEXT("HealAmount"),        Out.HealAmount);

	return !Out.ItemId.IsEmpty();
}
