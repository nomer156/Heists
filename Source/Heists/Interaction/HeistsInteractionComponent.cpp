// Copyright 2026 Heists. All Rights Reserved.

#include "Interaction/HeistsInteractionComponent.h"

#include "Interaction/HeistsInteractable.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UHeistsInteractionComponent::UHeistsInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHeistsInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHeistsInteractionComponent, ActiveInteractionTarget);
	DOREPLIFETIME(UHeistsInteractionComponent, ActiveActionId);
}

AActor* UHeistsInteractionComponent::FindBestInteractable() const
{
	const AActor* Owner = GetOwner();
	const UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return nullptr;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HeistsInteractionScan), false);
	QueryParams.AddIgnoredActor(Owner);

	if (!World->OverlapMultiByObjectType(
		Overlaps,
		Owner->GetActorLocation(),
		FQuat::Identity,
		ObjectParams,
		FCollisionShape::MakeSphere(InteractionRadius),
		QueryParams))
	{
		return nullptr;
	}

	AActor* BestActor = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || !Candidate->GetClass()->ImplementsInterface(UHeistsInteractable::StaticClass()))
		{
			continue;
		}

		const TArray<FHeistsInteractionAction> Actions =
			IHeistsInteractable::Execute_GetAvailableInteractionActions(Candidate, const_cast<AActor*>(Owner));
		if (Actions.IsEmpty())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(Owner->GetActorLocation(), Candidate->GetActorLocation());
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestActor = Candidate;
		}
	}

	return BestActor;
}

TArray<FHeistsInteractionAction> UHeistsInteractionComponent::GetAvailableActionsForTarget(AActor* Target) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !Target || !Target->GetClass()->ImplementsInterface(UHeistsInteractable::StaticClass()))
	{
		return {};
	}

	return IHeistsInteractable::Execute_GetAvailableInteractionActions(Target, Owner);
}

void UHeistsInteractionComponent::RequestPrimaryInteraction()
{
	AActor* Target = FindBestInteractable();
	if (!Target)
	{
		return;
	}

	const TArray<FHeistsInteractionAction> Actions = GetAvailableActionsForTarget(Target);
	for (const FHeistsInteractionAction& Action : Actions)
	{
		if (Action.bIsEnabled)
		{
			RequestInteraction(Target, Action.ActionId);
			return;
		}
	}
}

void UHeistsInteractionComponent::RequestInteraction(AActor* Target, EHeistsInteractionActionId ActionId)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Target || ActionId == EHeistsInteractionActionId::None)
	{
		return;
	}

	if (Owner->HasAuthority())
	{
		Server_RequestInteraction_Implementation(Target, ActionId);
		return;
	}

	Server_RequestInteraction(Target, ActionId);
}

bool UHeistsInteractionComponent::Server_RequestInteraction_Validate(AActor* Target, EHeistsInteractionActionId ActionId)
{
	return Target != nullptr && ActionId != EHeistsInteractionActionId::None;
}

void UHeistsInteractionComponent::Server_RequestInteraction_Implementation(AActor* Target, EHeistsInteractionActionId ActionId)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Target || !IsTargetInRange(Target) || !Target->GetClass()->ImplementsInterface(UHeistsInteractable::StaticClass()))
	{
		return;
	}

	const TArray<FHeistsInteractionAction> Actions =
		IHeistsInteractable::Execute_GetAvailableInteractionActions(Target, Owner);
	if (!IsActionAvailable(Actions, ActionId))
	{
		return;
	}

	if (!IHeistsInteractable::Execute_CanInteract(Target, Owner, ActionId))
	{
		return;
	}

	ActiveInteractionTarget = Target;
	ActiveActionId = ActionId;
	IHeistsInteractable::Execute_BeginInteract(Target, Owner, ActionId);
}

void UHeistsInteractionComponent::Server_CancelInteraction_Implementation()
{
	AActor* Owner = GetOwner();
	if (ActiveInteractionTarget && ActiveInteractionTarget->GetClass()->ImplementsInterface(UHeistsInteractable::StaticClass()))
	{
		IHeistsInteractable::Execute_CancelInteract(ActiveInteractionTarget, Owner);
	}

	ActiveInteractionTarget = nullptr;
	ActiveActionId = EHeistsInteractionActionId::None;
}

bool UHeistsInteractionComponent::IsTargetInRange(AActor* Target) const
{
	const AActor* Owner = GetOwner();
	return Owner && Target && FVector::DistSquared(Owner->GetActorLocation(), Target->GetActorLocation()) <= FMath::Square(InteractionRadius);
}

bool UHeistsInteractionComponent::IsActionAvailable(const TArray<FHeistsInteractionAction>& Actions, EHeistsInteractionActionId ActionId)
{
	for (const FHeistsInteractionAction& Action : Actions)
	{
		if (Action.ActionId == ActionId && Action.bIsEnabled)
		{
			return true;
		}
	}

	return false;
}
