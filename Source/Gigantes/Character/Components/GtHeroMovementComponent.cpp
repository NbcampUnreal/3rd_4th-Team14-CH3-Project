#include "GtHeroMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Gigantes/GtGameplayTags.h"
#include "Gigantes/Character/GtHeroCharacter.h"
#include "Gigantes/Player/GtPlayerCameraManager.h"

UGtHeroMovementComponent::UGtHeroMovementComponent()
{
    NavAgentProps.bCanCrouch = true;
    bCanWalkOffLedgesWhenCrouching = true;
 
}

void UGtHeroMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
    Super::SetUpdatedComponent(NewUpdatedComponent);

    HeroCharacterOwner = Cast<AGtHeroCharacter>(CharacterOwner);

    // 초기 가져와야할 캐릭터 정보들 캐싱
    if (CharacterOwner)
    {
        CacheInitialValues();
    }
}

void UGtHeroMovementComponent::CacheInitialValues()
{
    if (CharacterOwner && CharacterOwner->GetCapsuleComponent())
    {
        StandingCapsuleHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
    }
}

void UGtHeroMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
    
    // 슬라이딩 시작 시: 기본 값 저장 후 슬라이드 값으로 변경
    // TODO: 어차피 Custom 모드로 변경해서 PhysCustom을 호출하므로 굳이 필요 없을 것 같음
    if (MovementMode == MOVE_Custom && CustomMovementMode == CMM_Slide)
    {
        DefaultGroundFriction = GroundFriction;
        DefaultBrakingDecelerationWalking = BrakingDecelerationWalking;

        GroundFriction = 0.f; // PhysSlide에서 수동으로 감속을 처리하므로 기본 마찰은 0으로
        BrakingDecelerationWalking = SlideBrakingDeceleration;
    }
    // 슬라이딩 종료 시: 저장했던 기본 값으로 복원
    else if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMM_Slide)
    {
        GroundFriction = DefaultGroundFriction;
        BrakingDecelerationWalking = DefaultBrakingDecelerationWalking;
    }
    
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

void UGtHeroMovementComponent::StartSlide()
{
    if (!CharacterOwner || !CharacterOwner->GetCapsuleComponent())
        return;
    
    SetCapsuleSize(CrouchedHalfHeight);

    SetMovementMode(MOVE_Custom, CMM_Slide);

    // 속도 부스트
    Velocity *= SlideBoostMultiplier;
    
    // 최대 속도 제한
    float CurrentSpeed = Velocity.Size2D();
    if (CurrentSpeed > SlideMaxSpeed)
    {
        Velocity = Velocity.GetSafeNormal2D() * SlideMaxSpeed;
    }

}

void UGtHeroMovementComponent::EndSlide(ESlideEndReason Reason)
{
    if (!CharacterOwner || !CharacterOwner->GetCapsuleComponent())
        return;
    
    // 현재 속도 보존
    FVector PreSlideVelocity = Velocity;
    
    // Reason에 따른 처리
    switch(Reason)
    {
    case ESlideEndReason::Jump:
    case ESlideEndReason::Falling:
        {
            // 공중에 있는 경우 무조건 캡슐 복원 시도
            RestoreCapsuleSize();
            SetMovementMode(MOVE_Falling);
            break;
        }

    case ESlideEndReason::CrouchInput:
    case ESlideEndReason::Normal:
    case ESlideEndReason::Collision:
        {
            if (CanStandUp())
            {
                // 일어설 수 있으면 캡슐 복원
                RestoreCapsuleSize();
                SetMovementMode(MOVE_Walking);
            }
            else
            {
                // 일어설 수 없으면 Crouch 상태로 전환
                TransitionToCrouch();
                SetMovementMode(MOVE_Walking);
            }
            break;
        }
    }
    
    // 속도 복원
    Velocity = PreSlideVelocity;
}

bool UGtHeroMovementComponent::CanSlide() const
{
    // 지상에 있고 충분한 속도가 있는지 체크
    if (!IsMovingOnGround())
        return false;
        
    float CurrentSpeed = Velocity.Size2D();
    if (CurrentSpeed < SlideMinSpeed)
        return false;
        
    return true;
}

void UGtHeroMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
    Super::PhysCustom(DeltaTime, Iterations);
    if (CustomMovementMode == CMM_WallRun)
    {
        PhysWallRun(DeltaTime, Iterations);
    }
    else if (CustomMovementMode == CMM_Slide)
    {
        PhysSlide(DeltaTime, Iterations);
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
    
    if (AGtHeroCharacter* Hero = HeroCharacterOwner)
    {
        if (APlayerController* PC = Cast<APlayerController>(Hero->GetController()))
        {
            if (AGtPlayerCameraManager* CameraManager = Cast<AGtPlayerCameraManager>(PC->PlayerCameraManager))
            {
                const float CurrentAimOffsetYaw = CameraManager->GetAimOffsetYaw();
                
                // 월런 중 시선 체크 로직
                const bool bIsRightWall = FVector::DotProduct(WallRunNormal, UpdatedComponent->GetRightVector()) < 0;
                
                // 벽을 직접 바라보면 월런 종료
                if (bIsRightWall)
                {
                    // 오른쪽 벽: 왼쪽을 봐야 함
                    if (CurrentAimOffsetYaw < -100.0f || CurrentAimOffsetYaw > 50.0f)
                    {
                        EndWallRun();
                        return;
                    }
                }
                else
                {
                    // 왼쪽 벽: 오른쪽을 봐야 함  
                    if (CurrentAimOffsetYaw < -50.0f || CurrentAimOffsetYaw > 100.0f)
                    {
                        EndWallRun();
                        return;
                    }
                }
            }
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
}

void UGtHeroMovementComponent::PhysSlide(float DeltaTime, int32 Iterations)
{
    if (!CharacterOwner)
    {
        EndSlide(ESlideEndReason::Normal);
        return;
    }
    
    // Custom Mode에서는 CurrentFloor를 수동으로 업데이트 필요
    FFindFloorResult FloorResult;
    FindFloor(UpdatedComponent->GetComponentLocation(), FloorResult, false);
    CurrentFloor = FloorResult;
    
    // 이제 CurrentFloor 사용 가능
    if (!CurrentFloor.IsWalkableFloor())
    {
        EndSlide(ESlideEndReason::Falling);
        return;
    }

    // 현재 속도
    float CurrentSpeed = Velocity.Size2D();
    FVector CurrentDirection = Velocity.GetSafeNormal2D();
    
    // 바닥 정보 사용
    const FVector FloorNormal = CurrentFloor.HitResult.ImpactNormal;
    const float SlopeAngle = FMath::RadiansToDegrees(
        FMath::Acos(FVector::DotProduct(FloorNormal, FVector::UpVector)));
    
    // 경사면 처리
    if (SlopeAngle > 5.0f && SlopeAngle < GetWalkableFloorAngle())
    {
        // 경사 방향 계산
        FVector GravityDir = FVector(0, 0, -1);
        FVector RampDirection = (GravityDir - FloorNormal * 
            FVector::DotProduct(GravityDir, FloorNormal)).GetSafeNormal();
        
        // 경사 가속
        float SlopeAcceleration = FMath::Abs(GetGravityZ()) * SlideGravityScale * 
            FMath::Sin(FMath::DegreesToRadians(SlopeAngle));
        
        // 속도 증가
        CurrentSpeed += SlopeAcceleration * DeltaTime;
        
        // 방향을 경사면 방향으로 조정
        CurrentDirection = FMath::Lerp(CurrentDirection, RampDirection, 0.5f).GetSafeNormal();
    }
    else
    {
        // 평지 감속
        CurrentSpeed = FMath::Max(0.0f, CurrentSpeed - SlideBrakingDeceleration * DeltaTime);
        
        // 최소 속도 체크
        if (CurrentSpeed < SlideMinExitSpeed)
        {
            EndSlide(ESlideEndReason::Normal);
            return;
        }
    }
    
    // 속도 제한
    CurrentSpeed = FMath::Clamp(CurrentSpeed, 0.0f, SlideMaxSpeed);
    
    // 입력에 의한 조향 (TODO: 조향 대신 캐릭터와 Input의 방향을 비교하여 속도 계산 어떻게 할지 고민)
    const FVector InputVector = CharacterOwner->GetLastMovementInputVector();
    if (!InputVector.IsNearlyZero())
    {
        FVector InputDir = InputVector.GetSafeNormal();
        CurrentDirection = (CurrentDirection + InputDir * SlideSteeringStrength).GetSafeNormal();
    }
    
    // 속도 설정
    Velocity = CurrentDirection * CurrentSpeed;
    Velocity.Z = 0;
    
    // 이동 적용
    const FVector Delta = Velocity * DeltaTime;
    FHitResult Hit;
    SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);
    
    if (Hit.bBlockingHit)
    {
        SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
        
        // 정면 충돌
        if (FVector::DotProduct(CurrentDirection, Hit.Normal) < -0.7f)
        {
            EndSlide(ESlideEndReason::Collision);
        }
    }
}

bool UGtHeroMovementComponent::CanStandUp() const
{
    if (!CharacterOwner)
    {
        return false;
    }
    
    // 현재 웅크린 상태의 캡슐 정보
    UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
    if (!CapsuleComp)
    {
        return false;
    }
    
    const float CurrentCapsuleRadius = CapsuleComp->GetScaledCapsuleRadius();
    const float CurrentCapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
    
    // 일어섰을 때의 캡슐로 체크
    const float StandingHalfHeight = StandingCapsuleHalfHeight;
    const float HeightDifference = StandingHalfHeight - CurrentCapsuleHalfHeight;
    
    // 위쪽으로 체크
    FVector Start = CharacterOwner->GetActorLocation();
    FVector End = Start + FVector(0, 0, HeightDifference * 2.0f);
    
    FCollisionQueryParams QueryParams(NAME_None, false, CharacterOwner);
    FCollisionResponseParams ResponseParam;
    InitCollisionParams(QueryParams, ResponseParam);
    
    // 캡슐 모양으로 체크
    FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(CurrentCapsuleRadius, StandingHalfHeight);
    
    FHitResult Hit;
    bool bBlocked = GetWorld()->SweepSingleByChannel(
        Hit,
        Start,
        Start + FVector(0, 0, 0.1f),  // 거의 제자리에서 체크
        FQuat::Identity,
        UpdatedComponent->GetCollisionObjectType(),
        CapsuleShape,
        QueryParams,
        ResponseParam
    );
    
    return !bBlocked;
}

void UGtHeroMovementComponent::SetCapsuleSize(float TargetHalfHeight)
{
    UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
    if (!CapsuleComp)
        return;
    
    const float ComponentScale = CapsuleComp->GetShapeScale();
    const float OldUnscaledHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
    const float OldUnscaledRadius = CapsuleComp->GetUnscaledCapsuleRadius();
    
    // 크기 변경
    CapsuleComp->SetCapsuleSize(OldUnscaledRadius, TargetHalfHeight);
    float HalfHeightAdjust = (OldUnscaledHalfHeight - TargetHalfHeight);
    float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
    
    // 캡슐 위치 조정
    if (FMath::Abs(ScaledHalfHeightAdjust) > 0.01f)
    {
        UpdatedComponent->MoveComponent(
            FVector(0.f, 0.f, -ScaledHalfHeightAdjust), 
            UpdatedComponent->GetComponentQuat(), 
            true, nullptr, 
            EMoveComponentFlags::MOVECOMP_NoFlags, 
            ETeleportType::TeleportPhysics);
    }
    
    // 캡슐 크기 변경 델리게이트 호출
    OnCapsuleSizeChanged.ExecuteIfBound(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void UGtHeroMovementComponent::RestoreCapsuleSize()
{
    SetCapsuleSize(StandingCapsuleHalfHeight);
}

void UGtHeroMovementComponent::TransitionToCrouch()
{
    // MovementComponent의 Crouch 플래그 설정(다음 업데이트 때 Crouch 상태로 변경)
    bWantsToCrouch = true;
    
    if (HeroCharacterOwner)
    {
        // 여기서 캐릭터의 Crouch 상태를 직접 설정하여 crouch 진입에 대한 메쉬 offset 조정 로직 실행 방지
        HeroCharacterOwner->bIsCrouched = true;
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






