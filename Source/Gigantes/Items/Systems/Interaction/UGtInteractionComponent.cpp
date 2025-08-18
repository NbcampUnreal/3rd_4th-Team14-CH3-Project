#include "UGtInteractionComponent.h"

#include "../Interaction/GtInteractable.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UGtInteractionComponent::UGtInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 기본으로 자주 쓰는 대상 채널 등록 (원하면 BP에서 덮어쓰기)
	ExtraObjectChannels = {
		ECC_Pawn,
		ECC_PhysicsBody,
		ECC_WorldDynamic
	};
}

void UGtInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGtInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateFocus();
}

void UGtInteractionComponent::UpdateFocus()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	// 시점(카메라 or 컨트롤러 로테이션) 기준
	FVector EyeLoc; FRotator EyeRot;
	if (ACharacter* Char = Cast<ACharacter>(Owner))
	{
		if (const UCameraComponent* Cam = Char->FindComponentByClass<UCameraComponent>())
		{
			EyeLoc = Cam->GetComponentLocation();
			EyeRot = Cam->GetComponentRotation();
		}
		else if (AController* C = Char->GetController())
		{
			C->GetPlayerViewPoint(EyeLoc, EyeRot);
		}
		else
		{
			EyeLoc = Owner->GetActorLocation();
			EyeRot = Owner->GetActorRotation();
		}
	}
	else
	{
		EyeLoc = Owner->GetActorLocation();
		EyeRot = Owner->GetActorRotation();
	}

	const FVector End = EyeLoc + EyeRot.Vector() * TraceDistance;

	// ── (변경점) 여러 오브젝트 채널을 대상으로 하는 ObjectType 트레이스 ──
	FCollisionObjectQueryParams ObjParams;
	if (ExtraObjectChannels.Num() > 0)
	{
		for (auto Chan : ExtraObjectChannels)
		{
			ObjParams.AddObjectTypesToQuery(Chan);
		}
	}
	else
	{
		// 안전한 기본값
		ObjParams.AddObjectTypesToQuery(ECC_Pawn);
		ObjParams.AddObjectTypesToQuery(ECC_PhysicsBody);
		ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractTrace), /*bTraceComplex=*/false);
	Params.AddIgnoredActor(Owner);

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByObjectType(Hit, EyeLoc, End, ObjParams, Params);

#if ENABLE_DRAW_DEBUG
	DrawDebugLine(GetWorld(), EyeLoc, End, FColor::Cyan, false, 0.f, 0, 0.5f);
	if (bHit) DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 8.f, FColor::Yellow, false, 0.f);
#endif

	AActor* NewTarget = bHit ? Hit.GetActor() : nullptr;

	// 인터페이스 유효성 확인 및 포커스 처리
	if (NewTarget && NewTarget->GetClass()->ImplementsInterface(UGtInteractable::StaticClass()))
	{
		if (FocusedActor.Get() != NewTarget)
		{
			ClearFocus();
			FocusedActor = NewTarget;
			IGtInteractable::Execute_BeginFocus(NewTarget, GetOwner());
		}
	}
	else
	{
		ClearFocus();
	}
}

void UGtInteractionComponent::ClearFocus()
{
	if (AActor* Prev = FocusedActor.Get())
	{
		if (Prev->GetClass()->ImplementsInterface(UGtInteractable::StaticClass()))
		{
			IGtInteractable::Execute_EndFocus(Prev, GetOwner());
		}
	}
	FocusedActor = nullptr;
}

void UGtInteractionComponent::TryInteract()
{
	if (AActor* Target = FocusedActor.Get())
	{
		if (Target->GetClass()->ImplementsInterface(UGtInteractable::StaticClass()))
		{
			if (IGtInteractable::Execute_CanInteract(Target, GetOwner()))
			{
				IGtInteractable::Execute_Interact(Target, GetOwner());
			}
		}
	}
}