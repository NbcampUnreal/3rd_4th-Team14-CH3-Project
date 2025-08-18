// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gigantes/Gameplay/Damage/GtDamageable.h"
#include "GtDamageReceiverComponent.generated.h"

// 데미지 처리 완료 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageProcessed, const FGtDamageResult&, DamageResult);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GIGANTES_API UGtDamageReceiverComponent : public UActorComponent, public IGtDamageable
{
	GENERATED_BODY()

public:
	UGtDamageReceiverComponent();

	virtual bool ApplyDamage_Implementation(const FGtDamageInfo& DamageInfo, FGtDamageResult& OutDamageResult) override;

	UPROPERTY(BlueprintAssignable, Category = "Damage")
	FOnDamageProcessed OnDamageProcessed;
};
