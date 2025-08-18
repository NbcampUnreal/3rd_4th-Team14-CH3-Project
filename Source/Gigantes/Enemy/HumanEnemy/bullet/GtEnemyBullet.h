#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GtEnemyBullet.generated.h"

UCLASS()
class GIGANTES_API AGtEnemyBullet : public AActor
{
	GENERATED_BODY()

public:
	// 생성자
	AGtEnemyBullet();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
			   UPrimitiveComponent* OtherComp, FVector NormalImpulse, 
			   const FHitResult& Hit);

	// 총알 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float BulletDamage = 10.0f;

public:	
	// 충돌 영역
	UPROPERTY(VisibleAnywhere, Category="Components")
	class USphereComponent* CollisionComp;

	// 투사체 이동 처리
	UPROPERTY(VisibleAnywhere, Category="Movement")
	class UProjectileMovementComponent* ProjectileMovement;

	// 시각적 메시
	UPROPERTY(VisibleAnywhere, Category="Components")
	class UStaticMeshComponent* MeshComp;
};
