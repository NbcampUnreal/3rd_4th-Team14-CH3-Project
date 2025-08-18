#include "GtGrenadeItem.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AGtGrenadeItem::AGtGrenadeItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 콜리전
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComponent->InitSphereRadius(6.f);
	CollisionComponent->SetCollisionProfileName(TEXT("Projectile")); // Block WorldStatic/WorldDynamic, Pawn 등 원하는 대로
	SetRootComponent(CollisionComponent);

	// 스태틱
	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	GrenadeMesh->SetupAttachment(RootComponent);
	GrenadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌은 스피어가 담당
	GrenadeMesh->SetSimulatePhysics(false);                           // ProjectileMovement와 물리 동시 금지

	// 이동
	Projectile = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile"));
	Projectile->bRotationFollowsVelocity = true;
	Projectile->ProjectileGravityScale   = 1.0f;
	Projectile->InitialSpeed             = 0.f;     // 던질 때 세팅
	Projectile->MaxSpeed                 = 4000.f;
	Projectile->SetUpdatedComponent(CollisionComponent);

	// (선택) 히트 바인딩
	// CollisionComponent->OnComponentHit.AddDynamic(this, &AGtGrenadeItem::OnHit);
}

void AGtGrenadeItem::BeginPlay()
{
	Super::BeginPlay();
}

void AGtGrenadeItem::InitFromData(const FGtItemData& InData)
{
	Super::InitFromData(InData);

	Damage       = static_cast<float>(InData.Damage);
	DamageRadius = (InData.ExplosionRadius > 0.f) ? InData.ExplosionRadius : DamageRadius;
	FuseSeconds  = (InData.ExplosionDelay  > 0.f) ? InData.ExplosionDelay  : FuseSeconds;
}

void AGtGrenadeItem::Throw(const FVector& InVelocity)
{
	// 초기 속도 부여 (투척)
	if (Projectile)
	{
		Projectile->Velocity = InVelocity;
	}

	// 퓨즈 타이머 작동
	if (FuseSeconds > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			FuseTimer, this, &AGtGrenadeItem::Explode, FuseSeconds, false);
	}
	else
	{
		Explode();
	}
}

void AGtGrenadeItem::Explode()
{
	// 이펙트/사운드
	if (ExplosionFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this, ExplosionFX, GetActorLocation(), GetActorRotation());
	}
	if (ExplosionSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSFX, GetActorLocation());
	}

	ApplyExplosionDamage();

	// 폭발 후 제거
	Destroy();
}

void AGtGrenadeItem::ApplyExplosionDamage()
{

	TArray<AActor*> Ignore;
	UGameplayStatics::ApplyRadialDamage(
		this,
		Damage,
		GetActorLocation(),
		DamageRadius,
		DamageTypeClass ? DamageTypeClass.Get() : UDamageType::StaticClass(),
		Ignore,
		this, // DamageCauser
		GetInstigatorController(),
		true  // DoFullDamage: 내부 falloff 적용 안 함(필요 시 false + ApplyRadialDamageWithFalloff)
	);
}

// 	// UGameplayStatics::ApplyRadialDamageWithFalloff 함수를 사용하여 거리에 따라 피해량이 감소하도록 수정
// 	UGameplayStatics::ApplyRadialDamageWithFalloff(
// 	   this,
// 	   Damage,           // 최대 피해량
// 	   10.f,             // 최소 피해량 (가장 먼 거리의 대상에게 적용될 최소 피해량)
// 	   GetActorLocation(),
// 	   DamageRadius * 0.5f, // 내부 반경 (이 반경 내의 대상은 최대 피해를 받음)
// 	   DamageRadius,      // 외부 반경 (이 반경을 넘어가는 대상은 피해를 받지 않음)
// 	   UDamageType::StaticClass(),
// 	   TArray<AActor*>(), // 무시할 액터 목록 (필요에 따라 채움)
// 	   this,
// 	   GetInstigatorController(),
// 	   ECollisionChannel::ECC_Visibility // 피해 판정을 위한 충돌 채널
// 	);
