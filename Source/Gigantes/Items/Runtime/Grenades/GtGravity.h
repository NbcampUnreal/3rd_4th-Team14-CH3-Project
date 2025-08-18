#pragma once
#include "CoreMinimal.h"
#include "Gigantes/Items/Runtime/Grenades/GtGrenadeItem.h"
#include "Engine/EngineTypes.h"
#include "CollisionQueryParams.h"
#include "GtGravity.generated.h"

UCLASS()
class GIGANTES_API AGtGravityGrenade : public AGtGrenadeItem
{
	GENERATED_BODY()
	
public:
	AGtGravityGrenade();
	virtual void InitFromData(const FGtItemData& InData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gravity|Config")
	float GravityDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gravity|Config")
	float PullStrength = 250000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gravity|Config")
	float PullTickInterval = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gravity|Config")
	bool bAffectCharacters = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gravity|Config")
	bool bAffectPhysicsBodies = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gravity|Damage")
	bool bDamageOnEndOnly = true;

	bool bGravityActive = false;
	FTimerHandle GravityStepTimer;
	FTimerHandle GravityEndTimer;

	virtual void Explode() override;
	void GravityStep();
	void EndGravityField();
	void ApplyEndDamage();
};