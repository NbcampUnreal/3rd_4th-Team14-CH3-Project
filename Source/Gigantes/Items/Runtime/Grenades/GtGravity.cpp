#include "GtGravity.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/EngineTypes.h"    
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"

AGtGravityGrenade::AGtGravityGrenade()
{
    FuseSeconds         = 0.8f;
    DamageRadius        = 600.f;
    Damage              = 60.f;
    GravityDuration     = 3.0f;
    PullStrength        = 250000.f;
    PullTickInterval    = 0.05f;
    bDamageOnEndOnly    = true;
    bAffectCharacters   = true;
    bAffectPhysicsBodies= true;
}

void AGtGravityGrenade::InitFromData(const FGtItemData& InData)
{
    Super::InitFromData(InData);
    // 부모에서 ExplosionDelay/Radius/Damage 매핑 완료
}

void AGtGravityGrenade::Explode()
{
    bGravityActive = true;

    GetWorldTimerManager().SetTimer(
        GravityStepTimer,
        this,
        &AGtGravityGrenade::GravityStep,
        PullTickInterval,
        true,
        0.f
        );
    
    GetWorldTimerManager().SetTimer(
        GravityEndTimer,
        this,
        &AGtGravityGrenade::EndGravityField,
        GravityDuration,
        false
        );
}

void AGtGravityGrenade::GravityStep()
{
    if (!bGravityActive) return;

    const FVector Center = GetActorLocation();

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(GravityField), false, this);

    FCollisionObjectQueryParams ObjParams = FCollisionObjectQueryParams::AllObjects;

    // (pawn / PhysicsBody 검사) 최적화 -> 채널 좁히기
    ObjParams.AddObjectTypesToQuery(ECC_Pawn);
    ObjParams.AddObjectTypesToQuery(ECC_PhysicsBody);

    if (!GetWorld()->OverlapMultiByObjectType(
            Overlaps,
            Center,
            FQuat::Identity,
            ObjParams,
            FCollisionShape::MakeSphere(DamageRadius),
            Params))
    {
        return;
    }

    for (const FOverlapResult& Hit : Overlaps)
    {
        AActor* Target = Hit.GetActor();
        if (!Target || Target == this) continue;

        const FVector ToCenter = (Center - Target->GetActorLocation());
        const float Dist = FMath::Max(ToCenter.Size(), 1.f);
        const FVector PullDir = ToCenter / Dist;

        // 캐릭터 흡인
        if (bAffectCharacters)
        {
            if (ACharacter* Char = Cast<ACharacter>(Target))
            {
                if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
                {
                    const float MassApprox = 80.f; // 대충 질량 추정치
                    const FVector Impulse = PullDir * (PullStrength * PullTickInterval / MassApprox);
                    Move->AddImpulse(Impulse, true);
                }
            }
        }

        // 물리 바디 흡인
        if (bAffectPhysicsBodies)
        {
            if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Hit.GetComponent()))
            {
                if (Prim->IsSimulatingPhysics())
                {
                    // 가까울수록 강하게(간단한 감쇠)
                    const float Falloff = FMath::Clamp(1.f / (Dist * 0.01f), 0.05f, 4.f);
                    const FVector Force = PullDir * (PullStrength * Falloff);
                    Prim->AddForce(Force, NAME_None, true);
                }
            }
        }
    }

    // 필요 시 도트 데미지: 여기서 ApplyRadialDamageWithFalloff 호출해도 됨
}

void AGtGravityGrenade::EndGravityField()
{
    bGravityActive = false;
    GetWorldTimerManager().ClearTimer(GravityStepTimer);

    if (bDamageOnEndOnly)
    {
        ApplyEndDamage();
    }

    Destroy();
}

void AGtGravityGrenade::ApplyEndDamage()
{
    TArray<AActor*> Ignore;
    UGameplayStatics::ApplyRadialDamage(
        this,
        Damage,
        GetActorLocation(),
        DamageRadius,
        DamageTypeClass ? DamageTypeClass.Get() : UDamageType::StaticClass(),
        Ignore,
        this,
        GetInstigatorController(),
        true
    );
}