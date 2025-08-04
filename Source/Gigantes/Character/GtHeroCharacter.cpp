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
	// TODO: AimOffset을 위한 Tick으로써 추후 false 가능성 있음
	PrimaryActorTick.bCanEverTick = true;

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

void AGtHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CalculateAimOffset();
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
	const bool bIsWallRunning = HasStatusTag(GtGameplayTags::Status_Action_WallRunning);
	
	// 다른 코드의 JumpEvent 로직처럼, 현재 상태에 따라 분기
	if (bIsWallRunning && HeroMovementComponent)
	{
		// 월런 점프 로직
		const FVector LaunchVelocity = HeroMovementComponent->GetWallRunNormal() * HeroMovementComponent->WallRunJumpOffForce
			+ FVector::UpVector * HeroMovementComponent->JumpZVelocity;
		LaunchCharacter(LaunchVelocity, false, true);

		// 월런 상태 종료 (태그 제거는 OnMovementModeChanged에서 자동으로 처리될 수 있도록)
		HeroMovementComponent->EndWallRun();
	}
	else if (CanJump())
	{
		// 일반 점프 / 더블 점프
		Jump();
	}
}

bool AGtHeroCharacter::CanJumpInternal_Implementation() const
{
	return GetCharacterMovement()->IsMovingOnGround() || JumpCount < MaxJumpCount;
}

void AGtHeroCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	JumpCount++;

	StartWallRunCheck();
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

// TODO: AimOffset을 위한 계산 함수로써 추후 카메라 매니저 등에서 구현 대체
void AGtHeroCharacter::CalculateAimOffset()
{
	if (!Controller)
	{
		AimOffsetYaw = 0.0f;
		AimOffsetPitch = 0.0f;
		return;
	}
    
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator CharacterRotation = GetActorRotation();
    
	// 월런 중일 때만 AimOffset 계산
	if (HasStatusTag(GtGameplayTags::Status_Action_WallRunning))
	{
		AimOffsetYaw = FRotator::NormalizeAxis(ControlRotation.Yaw - CharacterRotation.Yaw);
		AimOffsetPitch = FRotator::NormalizeAxis(ControlRotation.Pitch);
	}
	else
	{
		// 일반 상태에서는 캐릭터가 회전하므로 Yaw는 0
		AimOffsetYaw = 0.0f;
		AimOffsetPitch = FRotator::NormalizeAxis(ControlRotation.Pitch);
	}
}


