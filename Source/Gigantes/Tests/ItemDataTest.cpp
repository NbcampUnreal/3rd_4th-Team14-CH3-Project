#include "Misc/AutomationTest.h"
#include "Gigantes/Items/Structs/FGtItemData.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"
#include "JsonObjectConverter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FItemDataJsonParseTest,
								 "Gigantes.Item.JSONToStruct",
								 EAutomationTestFlags::EditorContext |
								 EAutomationTestFlags::EngineFilter
								 )

bool FItemDataJsonParseTest::RunTest(const FString& Parameters)
{
	const FString TestJson = R"(
    {
        "ItemId": "rifle_ak47_01",
        "ItemName": "AK-47",
        "ItemType": "Weapon",
        "SubType": "Riffle",
        "Description": "Full auto assault rifle",
        "Damage": 30,
        "AmmoInMagazine": 30,
        "MaxAmmo": 30,
        "FireRate": 0.09,
        "ReloadTime": 2.2,
        "ExplosionRadius": 0.0,
        "ExplosionDelay": 0.0,
        "HealAmount": 0,
        "ItemTag": "Item.Weapon.Riffle"
    })";

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(TestJson);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		AddError(TEXT("JSON 파싱 실패"));
		return false;
	}

	FGtItemData ParsedData;
	bool bSuccess = FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), FGtItemData::StaticStruct(), &ParsedData, 0, 0);

	if (!bSuccess)
	{
		AddError(TEXT("FGtItemData 변환 실패"));
		return false;
	}

	TestEqual(TEXT("Damage == 30"), ParsedData.Damage, 30);
	TestEqual(TEXT("FireRate == 0.09"), ParsedData.FireRate, 0.09f);
	TestEqual(TEXT("ItemName == AK-47"), ParsedData.ItemName, FString("AK-47"));

	return true;
}