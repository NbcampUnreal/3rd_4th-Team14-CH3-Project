// Fill out your copyright notice in the Description page of Project Settings.


#include "GtTurretBase.h"

#include "AnimNodeEditModes.h"

// Sets default values
AGtTurretBase::AGtTurretBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TurretMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TurretMesh"));
	SetRootComponent(TurretMesh);
	
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetupAttachment(TurretMesh);
	Collision->SetGenerateOverlapEvents(true);
	
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AGtTurretBase::OnOverlapBegin);
	Collision->OnComponentEndOverlap.AddDynamic(this, &AGtTurretBase::OnOverlapEnd);

	bIsFindEnermy = false;
	FindEnemyActor = nullptr;
}

// Called when the game starts or when spawned
void AGtTurretBase::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(
	FindEnermyHandle,
	this,
	&AGtTurretBase::EnermySearch,
	1.0f,
	true
	);
}

void AGtTurretBase::EnermySearch()
{
	UE_LOG(LogTemp, Warning, TEXT("Turrent is running"));
	if (bIsFindEnermy)
	{
		if (FindEnemyActor)
		{
			LookAt(FindEnemyActor);
		}
	}
}

void AGtTurretBase::LookAt(AActor* Target)
{
	if (!Target) return;
    
	// 타겟을 향한 방향 벡터 계산
	FVector Direction = Target->GetActorLocation() - GetActorLocation();
	Direction.Z = 0; // 수평 회전만 적용 (필요에 따라 조정)
    
	// 방향 벡터를 회전값으로 변환
	FRotator NewRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
    
	// 터렛 회전 적용
	SetActorRotation(NewRotation);
}

void AGtTurretBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Turrent is Find Enermy"));
	bIsFindEnermy = true;
	FindEnemyActor = OtherActor;
}

void AGtTurretBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("Turrent lose Enermy"));
	bIsFindEnermy = false;
	FindEnemyActor = nullptr;
}


