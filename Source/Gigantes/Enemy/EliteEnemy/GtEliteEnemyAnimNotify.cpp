// Fill out your copyright notice in the Description page of Project Settings.

#include "GtEliteEnemyAnimNotify.h"
#include "GtEliteEnemy.h"


void UGtEliteEnemyAnimNotify::Notify(USkeletalMeshComponent* MeshComp,
						UAnimSequenceBase* Animation,
						const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (AGtEliteEnemy* Enemy = Cast<AGtEliteEnemy>(MeshComp->GetOwner()))
	{
		Enemy->ReAttack();
	}
}
