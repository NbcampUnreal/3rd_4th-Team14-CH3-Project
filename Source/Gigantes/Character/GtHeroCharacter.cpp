#include "GtHeroCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/GtHeroMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Gigantes/GtGameplayTags.h"
#include "Gigantes/Input/GtInputComponent.h"

AGtHeroCharacter::AGtHeroCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGtHeroMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	HeroMovementComponent = Cast<UGtHeroMovementComponent>(GetCharacterMovement());
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation = true;
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;
	
}

// Called when the game starts or when spawned
void AGtHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	OnStatusTagChanged.AddDynamic(this, &ThisClass::OnCharacterStatusTagChanged);
	LandedDelegate.AddDynamic(this, &ThisClass::OnLandedCallback);

	if (HeroMovementComponent)
	{
		HeroMovementComponent->OnCapsuleSizeChanged.BindUObject(this, &AGtHeroCharacter::HandleCapsuleSizeChanged);
	}
}

void AGtHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	checkf(InputConfigDataAsset, TEXT("Forgot to assign a valid data asset as input config"));

	const ULocalPlayer* LocalPlayer = GetController<APlayerController>()->GetLocalPlayer();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

	check(Subsystem);
	Subsystem->ClearAllMappings();
	Subsystem->AddMappingContext(InputConfigDataAsset->DefaultMappingContext, 0);

	UGtInputComponent* GtInputComponent = CastChecked<UGtInputComponent>(PlayerInputComponent);

	GtInputComponent->BindNativeInputAction(InputConfigDataAsset, GtGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
	GtInputComponent->BindNativeInputAction(InputConfigDataAsset, GtGameplayTags::InputTag_Look, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
	GtInputComponent->BindNativeInputAction(InputConfigDataAsset, GtGameplayTags::InputTag_Jump, ETriggerEvent::Started, this, &ThisClass::Input_Jump);
	GtInputComponent->BindNativeInputAction(InputConfigDataAsset, GtGameplayTags::InputTag_Crouch, ETriggerEvent::Started, this, &ThisClass::Input_Crouch);
}

void AGtHeroCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	if (GetController())
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, GetController()->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);

			AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f) 
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void AGtHeroCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	if (Value.X != 0.0f)
	{
		// X에는 Yaw 값이 있음:
		// - Camera에 대해 Yaw 적용
		AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		// Y에는 Pitch 값!
		double AimInversionValue = -Value.Y;
		AddControllerPitchInput(AimInversionValue);
	}
}

void AGtHeroCharacter::Input_Jump(const FInputActionValue& InputActionValue)
{
	// 슬라이딩 중 점프 시 슬라이드 캔슬하고 점프
	if (HasStatusTag(GtGameplayTags::Status_Action_Sliding))
	{
		const FVector PreSlideVelocity = GetVelocity();
        
		HeroMovementComponent->EndSlide(ESlideEndReason::Jump);
		
		if (CanJump())
		{
			// 현재 속도 방향으로 추가 추진력
			FVector JumpBoost = PreSlideVelocity.GetSafeNormal2D() * 200.0f;
			JumpBoost.Z = GetCharacterMovement()->JumpZVelocity;
			LaunchCharacter(JumpBoost, false, true);
			JumpCount++;
		}
		return;
	}
	
	// 웅크린 상태에서 점프 입력 시 웅크리기 해제만
	if (HasStatusTag(GtGameplayTags::Status_Action_Crouching))
	{
		UnCrouch();
		return;  
	}
	
	const bool bIsWallRunning = HasStatusTag(GtGameplayTags::Status_Action_WallRunning);
	
	// 현재 상태에 따라 벽 점프킥 or 일반 점프
	if (bIsWallRunning && HeroMovementComponent)
	{
		// 월런 점프 로직
		const FVector LaunchVelocity = HeroMovementComponent->GetWallRunNormal() * HeroMovementComponent->WallRunJumpOffForce
			+ FVector::UpVector * HeroMovementComponent->JumpZVelocity;
		LaunchCharacter(LaunchVelocity, false, true);
		JumpCount++;

		// 월런 상태 종료 (태그 제거는 OnMovementModeChanged에서 자동으로 처리될 수 있도록)
		HeroMovementComponent->EndWallRun();
	}
	else if (CanJump())
	{
		// 일반 점프 / 더블 점프
		Jump();
	}
}

void AGtHeroCharacter::Input_Crouch(const FInputActionValue& InputActionValue)
{
	if (HasStatusTag(GtGameplayTags::Status_Action_Sliding))
	{
		HeroMovementComponent->EndSlide(ESlideEndReason::CrouchInput);
		return;
	}
	
	if (HasStatusTag(GtGameplayTags::Status_Action_Crouching))
	{
		UnCrouch();
	}
	else if (ShouldStartSlide())
	{
		StartSlide();
	}
	else
	{
		Crouch();
	}
}

bool AGtHeroCharacter::CanJumpInternal_Implementation() const
{
	// 슬라이딩 중에는 점프 가능
	if (HasStatusTag(GtGameplayTags::Status_Action_Sliding))
	{
		return GetCharacterMovement()->IsMovingOnGround() || JumpCount < MaxJumpCount;
	}
    
	// 웅크린 상태에서는 점프 불가
	if (HasStatusTag(GtGameplayTags::Status_Action_Crouching))
	{
		return false;
	}
	
	return GetCharacterMovement()->IsMovingOnGround() || JumpCount < MaxJumpCount;
}

void AGtHeroCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	JumpCount++;

	// TODO : OnMovementModeChanged에서 처리하도록 변경하고 WallRunCoolTime 타이머 도입 해야함
	StartWallRunCheck();
}

void AGtHeroCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	AddStatusTag(GtGameplayTags::Status_Action_Crouching);
}

void AGtHeroCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	RemoveStatusTag(GtGameplayTags::Status_Action_Crouching);
}

bool AGtHeroCharacter::CanCrouch() const
{
	// 월런 상태에서는 Crouch 불가
	if (HasStatusTag(GtGameplayTags::Status_Action_WallRunning))
	{
		return false;
	}
    
	// 공중에서는 Crouch 불가
	if (GetCharacterMovement() && GetCharacterMovement()->IsFalling())
	{
		return false;
	}
    
	// 죽은 상태에서는 Crouch 불가
	if (HasStatusTag(GtGameplayTags::Status_Dead))
	{
		return false;
	}
	
	return Super::CanCrouch();
}

bool AGtHeroCharacter::ShouldStartSlide() const
{
	if (!HeroMovementComponent)
		return false;
    
	// MovementComponent에서 슬라이드 가능 여부 확인
	return HeroMovementComponent->CanSlide();
}

void AGtHeroCharacter::StartSlide()
{
	if (!HeroMovementComponent)
	{
		return;
	}
	// MovementComponent에 슬라이드 움직임 시작 요청
	HeroMovementComponent->StartSlide();
}

void AGtHeroCharacter::HandleCapsuleSizeChanged(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	// 메쉬 위치 조정
	RecalculateBaseEyeHeight();
    
	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
	if (GetMesh() && DefaultChar->GetMesh())
	{
		FVector& MeshRelativeLocation = GetMesh()->GetRelativeLocation_DirectMutable();
        
		// 캡슐이 작아지면 메쉬를 위로 올림
		if (HalfHeightAdjust > 0) // 캡슐이 작아짐
		{
			MeshRelativeLocation.Z = DefaultChar->GetMesh()->GetRelativeLocation().Z + HalfHeightAdjust;
		}
		// 캡슐을 복구할 때 메쉬를 원래의 캡슐에 대한 상대 좌표로 이동
		else 
		{
			MeshRelativeLocation.Z = DefaultChar->GetMesh()->GetRelativeLocation().Z;
		}
        
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
}

void AGtHeroCharacter::OnLandedCallback(const FHitResult& Hit)
{
	JumpCount = 0;
	GetWorldTimerManager().ClearTimer(WallRunCheckTimer);
}

void AGtHeroCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	// 슬라이드 시작 처리
	if (GetCharacterMovement()->MovementMode == MOVE_Custom && PreviousCustomMode != CMM_Slide)
	{
		if (HeroMovementComponent && HeroMovementComponent->GetCustomMovementMode() == CMM_Slide)
		{
			AddStatusTag(GtGameplayTags::Status_Action_Sliding);
			RemoveStatusTag(GtGameplayTags::Status_Action_Crouching);
		}
	}
    
	// 슬라이드 종료 처리
	if (PreviousCustomMode == CMM_Slide)
	{
		RemoveStatusTag(GtGameplayTags::Status_Action_Sliding);
        
		// Crouch 상태로 전환되었는지 확인
		if (GetCharacterMovement()->IsCrouching())
		{
			AddStatusTag(GtGameplayTags::Status_Action_Crouching);
		}
	}
	
	// 월런 종료 처리
	if (PreviousCustomMode == CMM_WallRun)
	{
		HeroMovementComponent->bOrientRotationToMovement = true;
		RemoveStatusTag(GtGameplayTags::Status_Action_WallRunning_Left);
		RemoveStatusTag(GtGameplayTags::Status_Action_WallRunning_Right);
		if (HeroMovementComponent->IsMovingOnGround())
		{
			HeroMovementComponent->StopMovementImmediately();
		}
	}

	// TODO : MovementMode == MOVE_Falling 조건만 체크해서 WallRunCheck 시작하도록 하고 WallRunCoolTime 타이머 도입 해야함
	if (PrevMovementMode == MOVE_Walking && GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		StartWallRunCheck();
	}
}

void AGtHeroCharacter::StartWallRunCheck()
{
	// 0.05초마다 CheckForWallRun 함수를 반복적으로 호출하는 타이머 설정
	GetWorldTimerManager().SetTimer(
		WallRunCheckTimer, 
		this, 
		&AGtHeroCharacter::CheckForWallRun, 
		0.05f, 
		true, 
		0.f
	);
}

void AGtHeroCharacter::CheckForWallRun()
{
	// 월런 상태가 아니고 공중에 있을 때만 탐색
	if (!HasStatusTag(GtGameplayTags::Status_Action_WallRunning) && HeroMovementComponent->IsFalling())
	{
		bool bWallRunPossible, bIsRightWall;
		HeroMovementComponent->TryEnterWallRun(bWallRunPossible, bIsRightWall);

		// 월런 성공 시
		if (bWallRunPossible)
		{
			// 수동 회전 제어 모드
			HeroMovementComponent->bOrientRotationToMovement = false;
			HeroMovementComponent->Velocity.Z = 0;
			HeroMovementComponent->SetMovementMode(MOVE_Custom, CMM_WallRun);
			const FGameplayTag WallRunTag = bIsRightWall ? GtGameplayTags::Status_Action_WallRunning_Right : GtGameplayTags::Status_Action_WallRunning_Left;
			AddStatusTag(WallRunTag);

			// 월런을 시작했으므로 더 이상 탐색할 필요가 없으니 타이머를 중지
			GetWorldTimerManager().ClearTimer(WallRunCheckTimer);
		}
	}
	else
	{
		// 이미 월런 중이거나 땅에 착지했다면, 탐색 타이머 중지
		GetWorldTimerManager().ClearTimer(WallRunCheckTimer);
	}
}

void AGtHeroCharacter::OnCharacterStatusTagChanged(const FGameplayTag& StatusTag, bool bAdded)
{
	const bool bIsWallRunTag = StatusTag.MatchesTag(GtGameplayTags::Status_Action_WallRunning);

	if (bIsWallRunTag && bAdded)
	{
		JumpCount = 0;
	}
}




