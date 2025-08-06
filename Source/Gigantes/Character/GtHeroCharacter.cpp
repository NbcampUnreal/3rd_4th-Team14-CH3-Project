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
			// Left/Right -> X 값에 들어있음:
			// MovementDirection은 현재 카메라의 RightVector를 의미함 (World-Space)
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);

			// - 내부적으로 MovementDirection * Value.X를 MovementComponent에 적용(더하기)해준다
			AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f) 
		{
			// 앞서 Left/Right와 마찬가지로 Forward/Backward를 적용한다
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
	// 웅크린 상태에서 점프 입력 시 웅크리기 해제만
	if (HasStatusTag(GtGameplayTags::Status_Action_Crouching))
	{
		UnCrouch();
		return;  // 점프는 하지 않음
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
	if (HasStatusTag(GtGameplayTags::Status_Action_Crouching))
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

bool AGtHeroCharacter::CanJumpInternal_Implementation() const
{
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

void AGtHeroCharacter::OnLandedCallback(const FHitResult& Hit)
{
	JumpCount = 0;

	GetWorldTimerManager().ClearTimer(WallRunCheckTimer);
}

void AGtHeroCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	// 1. 월런 상태에서 벗어날 때
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

	// 2. 절벽 등에서 걸어서 떨어졌을 때 
	if (PrevMovementMode == MOVE_Walking && GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		// 걸어서 떨어져 공중 상태가 되었으므로, 월런 탐색 시작
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




