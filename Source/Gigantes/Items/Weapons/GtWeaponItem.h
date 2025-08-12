#pragma once

#include "CoreMinimal.h"
#include "Gigantes/Items/Base/GtItembase.h"
#include "NiagaraSystem.h"
#include "GtWeaponItem.generated.h"

UCLASS()
class GIGANTES_API AGtWeaponItem : public AGtItemBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGtWeaponItem();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();
	UFUNCTION(BlueprintCallable, Category="Weapon")
	virtual void Reload();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Weapon", meta=(DisplayName="OnFire_BP"))
	void OnFire();
	virtual void OnFire_Implementation();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Weapon", meta=(DisplayName="OnFire_BP"))
	void OnReload();
	virtual void OnReload_Implementation();
	
	/** 히트스캔 판정 수행 */
	UFUNCTION(BlueprintCallable, Category="Weapon|Fire")
	void DoHitscan();

	/** 맞았을 때 BP 훅 (이펙트/사운드) */
	UFUNCTION(BlueprintImplementableEvent, Category="Weapon|Hit", meta=(DisplayName="OnHit_BP"))
	void OnHit(const FHitResult& Hit);

	// 머즐 플래시 재생
	UFUNCTION(BlueprintCallable, Category = "Weapon|VFX")
	void PlayMuzzleFlash();
	
	// 소켓 트랜스폼 가져오기
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FTransform GetMuzzleSocketTransform() const;

protected:

	virtual void InitFromData(const FGtItemData& data) override;
	bool IsOnFireCooldown() const;
	void StartFireCooldown();
	/** 발사 시작 위치 (머즐) */
	FVector GetShootOrigin() const;
	/** 발사 방향 (퍼짐 반영) */
	FVector GetShootDirection() const;
	/** 소유자 컨트롤러 */
	AController* GetOwnerController() const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	UStaticMeshComponent* WeaponMesh;

	// 원샷 스폰용 시스템 (권장)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|VFX")
	UNiagaraSystem* MuzzleFlashEffect = nullptr;

	// (옵션) 재활용 컴포넌트 방식 쓰고 싶을 때만 사용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon|VFX")
	UNiagaraComponent* MuzzleFlashComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Socket")
	FName MuzzleSocketName = TEXT("MuzzleFlashSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon", meta=(ClampMin="0.0"))
	float Damage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon", meta=(ClampMin="0.0"))
	float FireRate = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon", meta=(ClampMin="0.0"))
	float ReloadTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon", meta=(ClampMin="0"))
	int32 MaxAmmo = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon", meta=(ClampMin="0"))
	int32 AmmoInMagazine = 30;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	bool bIsReloading = false;

	/** 히트스캔 사거리 (cm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Hitscan", meta=(ClampMin="0.0"))
	float TraceRange = 10000.f;

	/** 탄 퍼짐 (도) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Hitscan", meta=(ClampMin="0.0"))
	float BulletSpreadDeg = 0.5f;

	/** 데미지 타입 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Damage")
	TSubclassOf<class UDamageType> DamageTypeClass;

	/** 충돌 채널 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Hitscan")
	TEnumAsByte<ECollisionChannel> HitScanChannel = ECC_Visibility;

private:
	FTimerHandle FireCooldownHandle;
	FTimerHandle ReloadTimerHandle;

};
