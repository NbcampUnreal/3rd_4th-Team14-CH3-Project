#pragma once
#include "CoreMinimal.h"
#include "Gigantes/Items/Runtime/Core/GtItemBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "GtGrenadeItem.generated.h"

class UNiagaraSystem;

UCLASS()
class GIGANTES_API AGtGrenadeItem : public AGtItemBase
{
	GENERATED_BODY()
	
public:
	AGtGrenadeItem();

	virtual void InitFromData(const FGtItemData& InData) override;

	UFUNCTION(BlueprintCallable, Category="Grenade")
	virtual void Throw(const FVector& InVelocity);

protected:

	UPROPERTY(VisibleDefaultsOnly, Category="Projectile")
	USphereComponent* CollisionComponent;
	
	UPROPERTY(VisibleAnywhere, Category="Grenade")
	UStaticMeshComponent* GrenadeMesh;

	UPROPERTY(VisibleAnywhere, Category="Grenade")
	UProjectileMovementComponent* Projectile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grenade|Config")
	float FuseSeconds = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grenade|Config")
	float DamageRadius = 400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grenade|Config")
	float Damage = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grenade|Config")
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(EditDefaultsOnly, Category="Grenade|VFX")
	UNiagaraSystem* ExplosionFX;

	UPROPERTY(EditDefaultsOnly, Category="Grenade|SFX")
	USoundBase* ExplosionSFX;

	FTimerHandle FuseTimer;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Grenade")
	virtual void Explode();

	/** 싱글 기준: 클라에서 바로 데미지 적용 */
	virtual void ApplyExplosionDamage();
};