// Fill out your copyright notice in the Description page of Project Settings.

#include "GtEliteEnemy.h"
#include "GtEliteEnemyHitboxNotify.h"




void UGtEliteEnemyHitboxNotify::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (AGtEliteEnemy* Enemy = Cast<AGtEliteEnemy>(MeshComp->GetOwner()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack Start"));
		//hit box on
	}
}

void UGtEliteEnemyHitboxNotify::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (AGtEliteEnemy* Enemy = Cast<AGtEliteEnemy>(MeshComp->GetOwner()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack End"));
		//hit box off
		Enemy->ReAttack();
	}
}