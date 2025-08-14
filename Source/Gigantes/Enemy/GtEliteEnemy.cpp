// Fill out your copyright notice in the Description page of Project Settings.

#include "GtEliteEnemy.h"
#include "Components/SphereComponent.h"

AGtEliteEnemy::AGtEliteEnemy()
{
	PrimaryActorTick.bCanEverTick = false;
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetupAttachment(SphereCollision);
	SphereCollision->SetGenerateOverlapEvents(true);
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AGtEliteEnemy::OnOverlapBegin);
	SetRootComponent(SphereCollision);
	
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(SphereCollision);
	
	HitBox_R = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_R"));
	HitBox_L = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_L"));
	
	
	HitBox_R->SetGenerateOverlapEvents(true);
	HitBox_R->SetupAttachment(SkeletalMesh, TEXT("lowerarm_r"));
	
	HitBox_L->SetGenerateOverlapEvents(true);
	HitBox_L->SetupAttachment(SkeletalMesh, TEXT("lowerarm_l"));

	BossState = EBossState::Idle;
	Patern = {1,2,3};
	FindEnemyActor = nullptr;
}

void AGtEliteEnemy::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("[Boss]Turn On"));
}

void AGtEliteEnemy::ChoiceAttack()
{
	if (Patern.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Boss]No available patterns"));
		return;
	}
	
	if (Patern.Num() >= 1)
	{
	UE_LOG(LogTemp, Warning, TEXT("[Boss]Choice Attack %i"), Patern.Num());
	int32 PaternChoice = FMath::RandRange(0, Patern.Num() - 1);
	int32 select = Patern[PaternChoice];
	GetWorldTimerManager().ClearTimer(PlayerTraceTimer);
	switch (select)
		{
		case 1:
			BossState = EBossState::Attack01;
			AttacktTmer01_On();
			break;
		case 2:
			BossState = EBossState::Attack02;
			AttacktTmer02_On();
			break;
		case 3:
			BossState = EBossState::Attack03;
			AttacktTmer03_On();
			break;
		default:
		//
		break;
		}
	}		
}

void AGtEliteEnemy::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
								   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Player"))
	{
	UE_LOG(LogTemp, Warning, TEXT("[BOSS] Find Player"));
	PlayerTraceTimerOn();
	FindEnemyActor = OtherActor;
	ChoiceAttack();
	}
}

void AGtEliteEnemy::PlayerTraceTimerOn()
{
	UE_LOG(LogTemp, Warning, TEXT("[Boss]Attack01"));
	GetWorldTimerManager().SetTimer(
		PlayerTraceTimer,
		this,
		&AGtEliteEnemy::PlayerTrace,
		0.03f,
		true
	);
}

void AGtEliteEnemy::AttacktTmer01_On()
{
	GetWorldTimerManager().ClearTimer(PlayerTraceTimer);
	UE_LOG(LogTemp, Warning, TEXT("[Boss]Attack01"));
	Patern.RemoveSingle(1);
	GetWorldTimerManager().SetTimer(
		AttacktTmer01,
		this,
		&AGtEliteEnemy::Attackt01,
		0.1f,
		false,
		5.0f
	);
}

void AGtEliteEnemy::AttacktTmer02_On()
{
	GetWorldTimerManager().ClearTimer(PlayerTraceTimer);
	UE_LOG(LogTemp, Warning, TEXT("[Boss]Attack02"));
	Patern.RemoveSingle(2);
	GetWorldTimerManager().SetTimer(
		AttacktTmer02,
		this,
		&AGtEliteEnemy::Attackt02,
		0.1f,
		false,
		5.0f
	);
}

void AGtEliteEnemy::AttacktTmer03_On()
{
	GetWorldTimerManager().ClearTimer(PlayerTraceTimer);
	UE_LOG(LogTemp, Warning, TEXT("[Boss]Attack03"));
	Patern.RemoveSingle(3);
	GetWorldTimerManager().SetTimer(
		AttacktTmer03,
		this,
		&AGtEliteEnemy::Attackt03,
		0.1f,
		false,
		5.0f
	);
}

void AGtEliteEnemy::PlayerTrace()
{
	UE_LOG(LogTemp, Warning, TEXT("[Boss]Chase Player"));
		FVector Direction = FindEnemyActor->GetActorLocation() - GetActorLocation();
		Direction.Z = 0;
		Direction.Normalize();

		// 목표 회전 계산
		FRotator TargetRotation = Direction.Rotation();

		// 현재 회전
		FRotator CurrentRotation = GetActorRotation();

		// DeltaTime을 받아서 보간 회전
		float RotationSpeed = 7.0f; // 숫자가 클수록 빠르게 회전
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), RotationSpeed);

		// 부드럽게 회전 적용
		SetActorRotation(NewRotation);
}


void AGtEliteEnemy::Attackt01()
{
	Patern.Add(1);
	ChoiceAttack();
	PlayerTraceTimerOn();
}
void AGtEliteEnemy::Attackt02()
{
	Patern.Add(2);
	ChoiceAttack();
	PlayerTraceTimerOn();
}
void AGtEliteEnemy::Attackt03()
{
	Patern.Add(3);
	ChoiceAttack();
	PlayerTraceTimerOn();
}