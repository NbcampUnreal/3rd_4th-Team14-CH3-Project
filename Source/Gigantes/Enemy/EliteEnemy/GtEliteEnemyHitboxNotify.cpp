// Fill out your copyright notice in the Description page of Project Settings.

#include "GtEliteEnemy.h"
#include "Components/BoxComponent.h"
#include "GtEliteEnemyHitboxNotify.h"




void UGtEliteEnemyHitboxNotify::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    if (AGtEliteEnemy* Enemy = Cast<AGtEliteEnemy>(MeshComp->GetOwner()))
    {
        if (Enemy->HitBox_R)
        {
            Enemy->HitBox_R->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            UE_LOG(LogTemp, Warning, TEXT("HitBox_R Enabled"));
        }
        if (Enemy->HitBox_L)
        {
            Enemy->HitBox_L->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            UE_LOG(LogTemp, Warning, TEXT("HitBox_L Enabled"));
        }
    }
}

void UGtEliteEnemyHitboxNotify::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (AGtEliteEnemy* Enemy = Cast<AGtEliteEnemy>(MeshComp->GetOwner()))
    {
        if (Enemy->HitBox_R)
        {
            Enemy->HitBox_R->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            UE_LOG(LogTemp, Warning, TEXT("HitBox_R Disabled"));
        }
        if (Enemy->HitBox_L)
        {
            Enemy->HitBox_L->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            UE_LOG(LogTemp, Warning, TEXT("HitBox_L Disabled"));
        }
    }
}