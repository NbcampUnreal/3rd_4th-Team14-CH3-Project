#include "GtEnemyBullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gigantes/Gameplay/Damage/GtDamageable.h"

AGtEnemyBullet::AGtEnemyBullet()
{
	PrimaryActorTick.bCanEverTick = false;
	
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(10.0f);
	CollisionComp->SetCollisionProfileName("BlockAllDynamic");
	RootComponent = CollisionComp;
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	
	InitialLifeSpan = 3.0f;
}

void AGtEnemyBullet::BeginPlay()
{
	Super::BeginPlay();

	// 충돌 시 이벤트 바인딩
	CollisionComp->IgnoreActorWhenMoving(GetOwner(), true);
	CollisionComp->OnComponentHit.AddDynamic(this, &AGtEnemyBullet::OnHit);

}

void AGtEnemyBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
						  UPrimitiveComponent* OtherComp, FVector NormalImpulse, 
						  const FHitResult& Hit)
{
	if (OtherActor && OtherActor != this && OtherComp)
	{
		if (OtherActor->Implements<UGtDamageable>())
		{
			FGtDamageInfo DamageInfo;
			DamageInfo.BaseDamage = BulletDamage;
			DamageInfo.DamageCauser = this;
			DamageInfo.Instigator = nullptr;
			DamageInfo.HitResultInfo = Hit;
    
			FGtDamageResult DamageResult;
			IGtDamageable::Execute_ApplyDamage(OtherActor, DamageInfo, DamageResult);

			UE_LOG(LogTemp, Warning, TEXT("[TestWeapon] Hit %s for %.1f damage"), 
				*Hit.GetActor()->GetName(), DamageResult.FinalDamage);
		}
		
		Destroy();
	}
}
