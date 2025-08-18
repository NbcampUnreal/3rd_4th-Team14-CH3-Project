// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GtEliteEnemyHitboxNotify.generated.h"
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"


/**
 * 
 */
UCLASS()
class GIGANTES_API UGtEliteEnemyHitboxNotify : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

};
