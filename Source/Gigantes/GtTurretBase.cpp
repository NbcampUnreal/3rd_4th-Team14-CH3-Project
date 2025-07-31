
#include "GtTurretBase.h"
#include "AnimNodeEditModes.h"

AGtTurretBase::AGtTurretBase()
{

	PrimaryActorTick.bCanEverTick = false;

	TurretMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TurretMesh"));
	SetRootComponent(TurretMesh);
	
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetupAttachment(TurretMesh);
	Collision->SetGenerateOverlapEvents(true);
	
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AGtTurretBase::OnOverlapBegin);
	Collision->OnComponentEndOverlap.AddDynamic(this, &AGtTurretBase::OnOverlapEnd);

	bIsFindEnermy = false;
	bIsReadyToAttack = false;
	FindEnemyActor = nullptr;
}


void AGtTurretBase::BeginPlay()
{
	Super::BeginPlay();
}

void AGtTurretBase::EnermySearchTimerReset()
{
	GetWorldTimerManager().SetTimer(
	FindEnermyHandle,
	this,
	&AGtTurretBase::LookAt,
	0.02f,
	false
	);
}

void AGtTurretBase::AttackTimerReset()
{
	GetWorldTimerManager().SetTimer(
	AttackReadyHandle,
	this,
	&AGtTurretBase::Attack,
	3.0f,
	false
	);
}

void AGtTurretBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
								   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Turrent is Find Enermy"));
	bIsFindEnermy = true;
	FindEnemyActor = OtherActor;
	EnermySearchTimerReset();
	AttackTimerReset();
}

void AGtTurretBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("Turrent lose Enermy"));
	bIsFindEnermy = false;
	FindEnemyActor = nullptr;
	GetWorldTimerManager().ClearTimer(AttackReadyHandle);
	GetWorldTimerManager().ClearTimer(FindEnermyHandle);
}

void AGtTurretBase::LookAt()
{
	if (!FindEnemyActor) return;
    
	// 타겟을 향한 방향 벡터 계산
	FVector Direction = FindEnemyActor->GetActorLocation() - GetActorLocation();
	Direction.Z = 0; // 수평 회전만 적용 (필요에 따라 조정)
    
	// 방향 벡터를 회전값으로 변환
	FRotator CurrentRotation = GetActorRotation();
	FRotator TargetRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 8.0f);
    
	// 터렛 회전 적용
	SetActorRotation(NewRotation);

	UE_LOG(LogTemp, Warning, TEXT("Turrent is running"));

	EnermySearchTimerReset();		
}

void AGtTurretBase::Attack()
{
	//about attack system
	//it works after 3second
	UE_LOG(LogTemp, Warning, TEXT("Turrent Attack Enermy"));
	bIsReadyToAttack = true;
	
	if (FindEnemyActor)
	{
		AttackTimerReset();
	}
	
}

