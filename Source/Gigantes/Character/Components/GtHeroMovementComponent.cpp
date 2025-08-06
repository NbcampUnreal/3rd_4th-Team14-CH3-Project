#include "GtHeroMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Gigantes/Character/GtHeroCharacter.h"

UGtHeroMovementComponent::UGtHeroMovementComponent()
{
}

void UGtHeroMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
    Super::SetUpdatedComponent(NewUpdatedComponent);

    HeroCharacterOwner = Cast<AGtHeroCharacter>(CharacterOwner);
}

void UGtHeroMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
    InvalidateGroundInfo();
}

void UGtHeroMovementComponent::TryEnterWallRun(bool& bOutWallRunIsPossible, bool& bOutIsRightWall)
{
    bOutWallRunIsPossible = false;
    bOutIsRightWall = false;

    // 1. 공중에 있는가?
    if (!IsFalling())
    {
        return;
    }

    // 2. 충분히 빠른가?
    if (Velocity.Size2D() < WallRunMinSpeed)
    {
        return;
    }

    // 3. 바닥에서 너무 가깝지 않은가? (바닥에서 바로 월런이 되는 현상 방지)
    FHitResult FloorHit;
    if (GetWorld()->LineTraceSingleByChannel(FloorHit, UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentLocation() + FVector::DownVector * WallRunMinHeight, ECC_Visibility))
    {
        return; // 최소 높이보다 낮은 곳에 바닥이 감지되면 월런 불가
    }

    // 4. 좌/우 벽 감지
    const FVector Start = UpdatedComponent->GetComponentLocation();
    const FVector Right = UpdatedComponent->GetRightVector();
    const FVector Left = -Right;
    const FVector Forward = UpdatedComponent->GetForwardVector();
    
    FHitResult WallHit;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    const FCollisionShape ProbeShape = FCollisionShape::MakeSphere(20.f);

    // 오른쪽 벽 감지
    if (GetWorld()->SweepSingleByChannel(WallHit,Start,Start + Right * 75.f,FQuat::Identity,ECC_Visibility,ProbeShape,QueryParams))
    {
        if (FMath::Abs(WallHit.ImpactNormal.Z) < 0.3f)
        {
            if (FVector::DotProduct(Forward, -WallHit.ImpactNormal) > 0.1f)
            {
                WallRunNormal = WallHit.ImpactNormal;
                bOutIsRightWall = true;
                bOutWallRunIsPossible = true;
                return;
            }
        }
    }

    // 왼쪽 벽 감지
    if (GetWorld()->SweepSingleByChannel(WallHit,Start,Start + Left * 75.f,FQuat::Identity,ECC_Visibility,ProbeShape,QueryParams))
    {
        if (FMath::Abs(WallHit.ImpactNormal.Z) < 0.3f)
        {
            if (FVector::DotProduct(Forward, -WallHit.ImpactNormal) > 0.1f)
            {
                WallRunNormal = WallHit.ImpactNormal;
                bOutIsRightWall = false;
                bOutWallRunIsPossible = true;
                return;
            }
        }
    }
}

void UGtHeroMovementComponent::EndWallRun(const FHitResult* FloorHitOption)
{
    // 월런 내부 상태 정리
    WallRunNormal = FVector::ZeroVector;

    // 바닥 히트가 있고 걷기 가능한 표면이면 → 걷기
    if (FloorHitOption && IsWalkable(*FloorHitOption))
    {
        SetMovementMode(MOVE_Walking);
        StopMovementImmediately();
        if (CharacterOwner)
        {
            // 점프카운트 리셋등 델리게이트로 처리
            CharacterOwner->Landed(*FloorHitOption);
        }
        return;
    }

    // 그 외(벽 소실/속도 저하/시선 이탈/점프 이탈 등) → 낙하
    SetMovementMode(MOVE_Falling);
}

void UGtHeroMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
    Super::PhysCustom(DeltaTime, Iterations);
    if (CustomMovementMode == CMM_WallRun)
    {
        PhysWallRun(DeltaTime, Iterations);
    }
}

void UGtHeroMovementComponent::PhysWallRun(float DeltaTime, int32 Iterations)
{
    // 벽 위치 확인 및 기본 설정
    // DotProduct가 음수면 오른쪽 벽, 양수면 왼쪽 벽
    const bool bIsOnRightWall = FVector::DotProduct(WallRunNormal, UpdatedComponent->GetRightVector()) < 0;
    const FVector TraceStart = UpdatedComponent->GetComponentLocation();
    const FVector TraceDirection = bIsOnRightWall ? UpdatedComponent->GetRightVector() : -UpdatedComponent->GetRightVector();

    const FCollisionShape ProbeShape = FCollisionShape::MakeSphere(20.f);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    FHitResult WallHit;

    // 벽 지속 감지
    if (!GetWorld()->SweepSingleByChannel(WallHit, TraceStart, TraceStart + TraceDirection * 90.f, 
        FQuat::Identity, ECC_Visibility, ProbeShape, QueryParams))
    {
        EndWallRun();
        return;
    }

    // 벽 각도 체크
    if (FMath::Abs(WallHit.ImpactNormal.Z) >= 0.2f)
    {
        EndWallRun();
        return;
    }

    WallRunNormal = WallHit.ImpactNormal;
    
    const float CurrentGroundDistance = GetGroundDistance();
    
    // 월런 중 바닥과의 최소 거리 체크
    if (CurrentGroundDistance < WallRunKeepMinHeight)
    {
        // 바닥이 충분히 가까우면 어차피 곧 착지할 것이므로
        // FloorHit 없이 EndWallRun 호출
        EndWallRun();
        return;
    }
    
    // 최소 속도 미달 시 월런 불가
    // 수평 속도가 최소 유지 속도(초기 속도의 60%) 미만이면 종료
    float HorizontalSpeed = FVector(Velocity.X, Velocity.Y, 0).Size();
    const float MinMaintainSpeed = FMath::Max(150.f, WallRunMinSpeed * 0.6f);
    if (HorizontalSpeed < MinMaintainSpeed)
    {
        EndWallRun();
        return;
    }
    
    // AimOffset 기반 시선 체크
    if (HeroCharacterOwner)
    {
        const float CurrentAimOffsetYaw = HeroCharacterOwner->GetAimOffsetYaw();
        
        // 월런 중인 벽의 방향 확인
        const bool bIsRightWall = FVector::DotProduct(WallRunNormal, UpdatedComponent->GetRightVector()) < 0;
        
        // 벽 반대편을 보고 있는지 체크
        bool bLookingAwayFromWall = false;
        
        if (bIsRightWall)
        {
            // 오른쪽 벽: 왼쪽을 봐야 함 (AimOffset이 음수)
            bLookingAwayFromWall = (CurrentAimOffsetYaw >= -100.0f && CurrentAimOffsetYaw <= 50.0f);
        }
        else
        {
            // 왼쪽 벽: 오른쪽을 봐야 함 (AimOffset이 양수)
            bLookingAwayFromWall = (CurrentAimOffsetYaw >= -50.0f && CurrentAimOffsetYaw <= 100.0f);
        }
        
        // 벽을 너무 직접적으로 보면 월런 종료
        if (!bLookingAwayFromWall)
        {
            UE_LOG(LogTemp, Warning, TEXT("AimOffset out of valid range. Yaw: %.1f"), CurrentAimOffsetYaw);
            EndWallRun();
            return;
        }
        
        // 디버그 표시
        if (GEngine)
        {
            FString WallSide = bIsRightWall ? TEXT("Right") : TEXT("Left");
            GEngine->AddOnScreenDebugMessage(2, 0.f, FColor::Green, 
                FString::Printf(TEXT("WallRun %s - AimOffset: %.1f"), *WallSide, CurrentAimOffsetYaw));
        }
    }
    
    
    // 월런 진행 방향 계산
    FVector WallDirection = FVector::CrossProduct(WallRunNormal, FVector::UpVector).GetSafeNormal();
    if (FVector::DotProduct(UpdatedComponent->GetForwardVector(), WallDirection) < 0.f)
    {
        WallDirection *= -1;
    }

    // 캐릭터의 회전 보간 처리
    if (CharacterOwner && CharacterOwner->Controller)
    {
        // 월런 방향으로의 목표 회전값 계산
        FRotator TargetRotation = WallDirection.Rotation();
        FRotator CurrentRotation = CharacterOwner->GetActorRotation();
        
        // 부드러운 회전을 위한 보간
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, WallRunRotationSpeed);
        
        // Yaw만 적용 (Pitch와 Roll은 0으로 유지)
        NewRotation.Pitch = 0.0f;
        NewRotation.Roll = 0.0f;
        
        CharacterOwner->SetActorRotation(NewRotation);
    }
    
    // 입력 처리
    const FVector InputVector = CharacterOwner ? CharacterOwner->GetLastMovementInputVector().GetSafeNormal() : FVector::ZeroVector;
    const float ForwardInputAmount = FVector::DotProduct(InputVector, WallDirection);
    
    // 가속/감속 적용
    // 월런 방향과 정방향인 Input 방향에 대해 가속 적용
    if (ForwardInputAmount > 0.1f)
    {
        HorizontalSpeed += WallRunAcceleration * ForwardInputAmount * DeltaTime;
        HorizontalSpeed = FMath::Min(HorizontalSpeed, WallRunMaxSpeed);
    }
    // 월런 방향과 반대 방향 Input 또는 Input이 없을때 감속 적용
    else
    {
        HorizontalSpeed = FMath::Max(0.f, HorizontalSpeed - WallRunBrakingDeceleration * DeltaTime);
    }
    
    // 중력 적용 로직: 월런 중 감소된 중력 적용
    // 현재 Z 속도에 스케일된 중력을 누적
    // v = u + at 공식 적용(초기 속도 + 가속도 * 시간)
    float NewZVelocity = Velocity.Z + (GetGravityZ() * WallRunGravityScale * DeltaTime);
    
    // 새로운 속도 벡터 구성
    // 수평 속도와 수직 속도를 독립적으로 계산하여 수직 이동은 오직 중력에 의해서만 결정되도록 유도
    FVector NewVelocity = WallDirection * HorizontalSpeed;
    NewVelocity.Z = NewZVelocity;
    
    // 벽과의 거리 보정
    // 이상적 거리 = 캡슐 반지름 + 오프셋(현재 거리와의 차이를 50%만 즉시 보정하여 부드러운 움직임 표현)
    const float CapsuleRadius = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const float IdealDistance = CapsuleRadius + WallRunOffsetDistance;
    const float CurrentDistance = FVector::DotProduct(WallHit.ImpactPoint - TraceStart, -WallRunNormal);
    const float DistanceDelta = IdealDistance - CurrentDistance;
    
    // 보정을 Delta에만 적용 (Velocity에는 영향 없음)
    const FVector CorrectionDelta = WallRunNormal * DistanceDelta * 0.5f; // 50% 보정(property화 가능)
    
    // 이동 적용
    const FVector MoveDelta = NewVelocity * DeltaTime + CorrectionDelta;

    // 충돌 검사를 포함하여 안전하게 캐릭터 이동
    FHitResult Hit;
    SafeMoveUpdatedComponent(MoveDelta, UpdatedComponent->GetComponentQuat(), true, Hit);
    
    // Hit 처리
    if (Hit.bBlockingHit)
    {
        // 중간에 장애물 있을시 미끄러지듯 내려가도록 처리
        SlideAlongSurface(MoveDelta, 1.f - Hit.Time, Hit.Normal, Hit, true);
    }

    // 다음 프레임을 위한 속도 저장
    // Z 속도가 제대로 보존되도록 유도
    Velocity = NewVelocity;
    
    // 디버그 출력
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Yellow, 
            FString::Printf(TEXT("WallRun - Vel.Z: %.2f, HSpeed: %.2f, GravScale: %.2f"), 
            Velocity.Z, HorizontalSpeed, WallRunGravityScale));
    }
}

float UGtHeroMovementComponent::GetGroundDistance()
{
    // 이미 이번 프레임에 계산했으면 캐시 반환
    if (GFrameCounter == CachedGroundInfoFrame)
    {
        return CachedGroundDistance;
    }
    
    // 새로 계산
    CachedGroundDistance = CalculateGroundDistance();
    CachedGroundInfoFrame = GFrameCounter;
    
    return CachedGroundDistance;
}

float UGtHeroMovementComponent::CalculateGroundDistance() const
{
    if (!CharacterOwner)
    {
        return 0.0f;
    }
    
    // 땅에 있으면 0
    if (IsMovingOnGround())
    {
        return 0.0f;
    }
    
    // 공중에 있을 때만 거리 계산
    const UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
    if (!CapsuleComp)
    {
        return GroundTraceDistance;
    }
    
    const float CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
    const FVector TraceStart = CharacterOwner->GetActorLocation();
    const FVector TraceEnd = TraceStart - FVector(0, 0, GroundTraceDistance + CapsuleHalfHeight);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams(NAME_None, false, CharacterOwner);
    QueryParams.bReturnPhysicalMaterial = false;
    
    // 바닥 체크
    if (GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams))
    {
        // 바닥을 찾았으면 거리 계산
        return FMath::Max(0.0f, HitResult.Distance - CapsuleHalfHeight);
    }
    
    // 바닥을 못 찾았으면 최대 거리
    return GroundTraceDistance;
}

void UGtHeroMovementComponent::InvalidateGroundInfo()
{
    CachedGroundInfoFrame = 0;
}





