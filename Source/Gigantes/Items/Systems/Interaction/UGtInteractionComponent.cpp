#include "UGtInteractionComponent.h"

#include "Items/Systems/Interaction/GtInteractable.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UGtInteractionComponent::UGtInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 기본 대상으로 자주 쓰는 ObjectType들(원하면 BP에서 덮어써)
	if (ExtraObjectChannels.Num() == 0)
	{
		ExtraObjectChannels = { ECC_WorldDynamic, ECC_PhysicsBody, ECC_Pawn };
	}
}

void UGtInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGtInteractionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateFocus();
}

void UGtInteractionComponent::UpdateFocus()
{
	ACharacter* C = Cast<ACharacter>(GetOwner());
	if (!C)
	{
		ClearFocus();
		return;
	}

	// ── Start/End 계산: 컨트롤러가 있으면 뷰포인트 기준, 없으면 캐릭터 위치 기준
	FVector Start; FRotator Rot;
	if (APlayerController* PC = Cast<APlayerController>(C->GetController()))
	{
		PC->GetPlayerViewPoint(Start, Rot);
	}
	else
	{
		Start = C->GetActorLocation() + FVector(0, 0, 60.f);
		Rot   = C->GetActorRotation();
	}
	const FVector End = Start + Rot.Vector() * TraceDistance;

	// ── 라인트레이스(ObjectType 우선, 비어있으면 채널 폴백)
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(GtInteractTrace), false, C);
	Params.AddIgnoredActor(C); // 자기 자신 무시

	bool bHit = false;
	if (ExtraObjectChannels.Num() > 0)
	{
		FCollisionObjectQueryParams Obj;
		for (auto Ch : ExtraObjectChannels) { Obj.AddObjectTypesToQuery(Ch); }
		bHit = GetWorld()->LineTraceSingleByObjectType(Hit, Start, End, Obj, Params);
	}
	else
	{
		bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, Params);
	}

	AActor* NewTarget = bHit ? Hit.GetActor() : nullptr;

	// ── 포커스 전환 가드 + 인터페이스 체크
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
	if (FocusedActor.IsValid() && FocusedActor->GetClass()->ImplementsInterface(UGtInteractable::StaticClass()))
	{
		IGtInteractable::Execute_EndFocus(FocusedActor.Get(), GetOwner());
	}
	FocusedActor.Reset();
}

void UGtInteractionComponent::TryInteract()
{
	AActor* Target = FocusedActor.Get();
	if (Target && Target->GetClass()->ImplementsInterface(UGtInteractable::StaticClass()))
	{
		if (IGtInteractable::Execute_CanInteract(Target, GetOwner()))
		{
			IGtInteractable::Execute_Interact(Target, GetOwner());
		}
	}
}