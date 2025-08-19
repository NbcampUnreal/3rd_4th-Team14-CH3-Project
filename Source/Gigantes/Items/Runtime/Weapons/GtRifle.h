#pragma once
#include "CoreMinimal.h"
#include "Gigantes/Items/Runtime/Weapons/GtWeaponItem.h"
#include "GtRifle.generated.h"

/** 발사 모드 */
UENUM(BlueprintType)
enum class ERifleFireMode : uint8
{
    Semi    UMETA(DisplayName="Semi-Auto"),
    Burst   UMETA(DisplayName="Burst"),
    Auto    UMETA(DisplayName="Full-Auto")
};

UCLASS()
class GIGANTES_API AGtRifle : public AGtWeaponItem
{
    GENERATED_BODY()
public:
    AGtRifle();

    /* 덮어쓰기 */
    virtual void InitFromData(const FGtItemData& InData) override;

    /** 캐릭터 입력에서 바인딩: 눌렀을 때 */
    UFUNCTION(BlueprintCallable, Category="Weapon|Rifle")
    void StartTrigger();

    /** 캐릭터 입력에서 바인딩: 뗐을 때 */
    UFUNCTION(BlueprintCallable, Category="Weapon|Rifle")
    void ReleaseTrigger();

    /** 발사 모드 변경 (UI나 키 바인딩에서 호출) */
    UFUNCTION(BlueprintCallable, Category="Weapon|Rifle")
    void ToggleFireMode();

protected:
    /** 기본 파라미터(에디터에서 튜닝 가능, 데이터로 덮어쓰기 가능) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Rifle")
    ERifleFireMode FireMode = ERifleFireMode::Auto;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Rifle|Spread")
    float BaseSpreadDeg = 0.6f;          // 기본 탄퍼짐

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Rifle|Spread")
    float MaxSpreadDeg = 4.0f;           // 최대 탄퍼짐

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Rifle|Spread")
    float SpreadPerShotDeg = 0.25f;      // 발사당 누적

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Rifle|Spread")
    float SpreadRecoveryPerSec = 2.0f;   // 초당 회복(발사 중/중지 시 틱으로 회복)

    /** 버스트 설정 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Rifle|Burst")
    int32 BurstCount = 3;                // 한 번에 3발

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Rifle|Burst")
    float BurstInterval = 0.09f;         // 버스트 내 간격(초)

    /** 리코일 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Rifle|Recoil")
    float RecoilPitchPerShot = -0.6f;    // 위로 차오르게 음수(언리얼 피치축)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Rifle|Recoil")
    float RecoilYawJitterPerShot = 0.4f; // 좌우 랜덤

    /** ADS(줌) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Rifle|ADS")
    bool bUseADSZoom = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Rifle|ADS", meta=(EditCondition="bUseADSZoom"))
    float ADSFOV = 70.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Rifle|ADS", meta=(EditCondition="bUseADSZoom"))
    float ADSRestoreFOV = 90.f;

    /** 현재 누적 스프레드 */
    float CurrentSpreadDeg = 0.f;

    /** 입력 상태 */
    bool bHoldingTrigger = false;

    /** 타이머 */
    FTimerHandle AutoTimer;
    FTimerHandle BurstTimer;

    /** 버스트 진행 상태 */
    int32 ShotsLeftInBurst = 0;

    /** 부모에서 Fire() → OnFire 호출한다고 가정. 한 발 처리(히트스캔) */
    virtual void OnFire_Implementation() override;

    /** 내부: 히트스캔 한 발 */
    bool DoHitscanShot(float InDamage, float InRange, float InSpreadDeg);

    /** 내부: 자동 사격 루프 / 버스트 루프 */
    void AutoFireTick();
    void BurstFireTick();

    /** 스프레드 회복(발사 중에도 틱마다 조금씩 회복) */
    virtual void Tick(float DeltaSeconds) override;
    virtual void BeginPlay() override;

    /** 카메라 FOV */
    void ApplyADS(bool bEnable);

    /** 리코일 카메라 펀치 */
    void ApplyRecoil();
};