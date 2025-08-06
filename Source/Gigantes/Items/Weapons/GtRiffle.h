// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GtWeaponItem.h"
#include "GtRiffle.generated.h"

UCLASS()
class GIGANTES_API AGtRiffle : public AGtWeaponItem
{
	GENERATED_BODY()

public:
	virtual void Fire() override;
	virtual void Reload() override;
	virtual void InitFromData(const FGtItemData& data) override;
};
