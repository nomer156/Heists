// Copyright 2026 Heists. All Rights Reserved.

#include "Interaction/HeistsInteractionComponent.h"

#include "Character/HeistsRobber.h"
#include "GameFramework/Pawn.h"
#include "Interaction/HeistsInteractableActorBase.h"
#include "Interaction/HeistsInteractable.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UHeistsInteractionComponent::UHeistsInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
	SetIsReplicatedByDefault(true);
}

void UHeistsInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	UpdateLocalFocus();
}

void UHeistsInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateLocalFocus();
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

FHeistsInteractionAction UHeistsInteractionComponent::GetBestContextualActionForTarget(AActor* Target) const
{
	const TArray<FHeistsInteractionAction> Actions = GetAvailableActionsForTarget(Target);
	if (Actions.IsEmpty())
	{
		return {};
	}

	const auto FindEnabledAction = [&Actions](EHeistsInteractionActionId ActionId) -> const FHeistsInteractionAction*
	{
		return Actions.FindByPredicate([ActionId](const FHeistsInteractionAction& Action)
		{
			return Action.ActionId == ActionId && Action.bIsEnabled;
		});
	};

	const AHeistsRobber* Robber = Cast<AHeistsRobber>(GetOwner());
	const EHeistsCrewRole CrewRole = Robber ? Robber->GetCrewRole() : EHeistsCrewRole::None;
	switch (CrewRole)
	{
	case EHeistsCrewRole::Hacker:
		if (const FHeistsInteractionAction* HackAction = FindEnabledAction(EHeistsInteractionActionId::Hack))
		{
			return *HackAction;
		}
		break;
	case EHeistsCrewRole::Breaker:
		if (const FHeistsInteractionAction* BreachAction = FindEnabledAction(EHeistsInteractionActionId::Breach))
		{
			return *BreachAction;
		}
		break;
	case EHeistsCrewRole::Scout:
		for (const EHeistsInteractionActionId PreferredAction : { EHeistsInteractionActionId::Peek, EHeistsInteractionActionId::Open, EHeistsInteractionActionId::Search })
		{
			if (const FHeistsInteractionAction* ScoutAction = FindEnabledAction(PreferredAction))
			{
				return *ScoutAction;
			}
		}
		break;
	case EHeistsCrewRole::Coordinator:
		for (const EHeistsInteractionActionId PreferredAction : { EHeistsInteractionActionId::Inspect, EHeistsInteractionActionId::Activate })
		{
			if (const FHeistsInteractionAction* CoordinatorAction = FindEnabledAction(PreferredAction))
			{
				return *CoordinatorAction;
			}
		}
		break;
	default:
		break;
	}

	for (const FHeistsInteractionAction& Action : Actions)
	{
		if (Action.bIsEnabled)
		{
			return Action;
		}
	}

	return {};
}

float UHeistsInteractionComponent::GetRoleAdjustedActionDuration(const FHeistsInteractionAction& Action) const
{
	const AHeistsRobber* Robber = Cast<AHeistsRobber>(GetOwner());
	if (!Robber)
	{
		return Action.Duration;
	}

	const FHeistsRoleTuning RoleTuning = Robber->GetRoleTuning();
	float Multiplier = 1.f;

	switch (Action.ActionId)
	{
	case EHeistsInteractionActionId::Hack:
	case EHeistsInteractionActionId::ScanFingerprint:
	case EHeistsInteractionActionId::TimingInput:
		Multiplier = RoleTuning.HackDurationMultiplier;
		break;
	case EHeistsInteractionActionId::Breach:
	case EHeistsInteractionActionId::Lock:
	case EHeistsInteractionActionId::Unlock:
		Multiplier = RoleTuning.ForceDurationMultiplier;
		break;
	case EHeistsInteractionActionId::Open:
	case EHeistsInteractionActionId::Close:
	case EHeistsInteractionActionId::Peek:
	case EHeistsInteractionActionId::Search:
		Multiplier = RoleTuning.QuietInteractionMultiplier;
		break;
	default:
		break;
	}

	return FMath::Max(0.f, Action.Duration * Multiplier);
}

void UHeistsInteractionComponent::RequestPrimaryInteraction()
{
	AActor* Target = FindBestInteractable();
	if (!Target)
	{
		return;
	}

	const FHeistsInteractionAction BestAction = GetBestContextualActionForTarget(Target);
	if (BestAction.bIsEnabled && BestAction.ActionId != EHeistsInteractionActionId::None)
	{
		RequestInteraction(Target, BestAction.ActionId);
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

void UHeistsInteractionComponent::UpdateLocalFocus()
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	AActor* NewFocusedActor = FindBestInteractable();
	AActor* PreviousFocusedActor = FocusedInteractable.Get();
	if (PreviousFocusedActor == NewFocusedActor)
	{
		return;
	}

	SetActorLocalFocus(PreviousFocusedActor, false);
	FocusedInteractable = NewFocusedActor;
	SetActorLocalFocus(NewFocusedActor, true);
}

void UHeistsInteractionComponent::SetActorLocalFocus(AActor* Actor, bool bFocused)
{
	if (AHeistsInteractableActorBase* InteractableActor = Cast<AHeistsInteractableActorBase>(Actor))
	{
		InteractableActor->SetLocallyFocused(bFocused);
	}
}
