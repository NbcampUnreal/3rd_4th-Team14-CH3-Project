// Fill out your copyright notice in the Description page of Project Settings.

#include "GtEliteEnemy.h"
#include "GtEliteEnemyAnimNotify.h"

void UGtEliteEnemyAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	if (AGtEliteEnemy* Enemy = Cast<AGtEliteEnemy>(MeshComp->GetOwner()))
	{
		Enemy->ReAttack();
	}
}
