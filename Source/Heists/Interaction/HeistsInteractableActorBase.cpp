// Copyright 2026 Heists. All Rights Reserved.

#include "Interaction/HeistsInteractableActorBase.h"

#include "Game/HeistsGameState.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AHeistsInteractableActorBase::AHeistsInteractableActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);

	DebugMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugMesh"));
	SetRootComponent(DebugMesh);
	DebugMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DebugMesh->SetCollisionObjectType(ECC_WorldDynamic);
	DebugMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	DebugMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DebugMesh->SetGenerateOverlapEvents(true);
	DebugMesh->SetRenderCustomDepth(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshFinder.Succeeded())
	{
		DebugMesh->SetStaticMesh(CubeMeshFinder.Object);
	}
}

void AHeistsInteractableActorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHeistsInteractableActorBase, ActiveInteractor);
	DOREPLIFETIME(AHeistsInteractableActorBase, InteractionProgress);
	DOREPLIFETIME(AHeistsInteractableActorBase, ActiveActionId);
}

TArray<FHeistsInteractionAction> AHeistsInteractableActorBase::GetAvailableInteractionActions_Implementation(AActor* Interactor) const
{
	return Actions;
}

void AHeistsInteractableActorBase::SetLocallyFocused(bool bFocused)
{
	if (bIsLocallyFocused == bFocused)
	{
		return;
	}

	bIsLocallyFocused = bFocused;
	ApplyLocalFocusVisualState();
}

bool AHeistsInteractableActorBase::CanInteract_Implementation(AActor* Interactor, EHeistsInteractionActionId ActionId) const
{
	if (!Interactor || (ActiveInteractor && ActiveInteractor != Interactor))
	{
		return false;
	}

	const FHeistsInteractionAction* Action = FindAction(ActionId);
	if (!Action || !Action->bIsEnabled)
	{
		return false;
	}

	if (Action->bRequiresSharedItem)
	{
		const AHeistsGameState* HeistsGameState = GetWorld() ? GetWorld()->GetGameState<AHeistsGameState>() : nullptr;
		return HeistsGameState && HeistsGameState->HasSharedCrewItem(Action->RequiredSharedItem);
	}

	return true;
}

void AHeistsInteractableActorBase::BeginInteract_Implementation(AActor* Interactor, EHeistsInteractionActionId ActionId)
{
	if (!HasAuthority() || !CanInteract_Implementation(Interactor, ActionId))
	{
		return;
	}

	const FHeistsInteractionAction* Action = FindAction(ActionId);
	if (!Action)
	{
		return;
	}

	ActiveInteractor = Interactor;
	ActiveActionId = ActionId;
	InteractionProgress = 0.f;
	ActiveInteractionDuration = Action->Duration;
	BP_OnBusyStateChanged();
	BP_OnProgressChanged();

	if (Action->TaskType == EHeistsInteractionTaskType::Instant || Action->Duration <= 0.f)
	{
		CommitInteract_Implementation(Interactor, ActionId);
		return;
	}

	GetWorldTimerManager().SetTimer(
		InteractionProgressTimerHandle,
		this,
		&AHeistsInteractableActorBase::AdvanceInteractionProgress,
		0.1f,
		true);
}

void AHeistsInteractableActorBase::CancelInteract_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || (ActiveInteractor && ActiveInteractor != Interactor))
	{
		return;
	}

	ClearInteractionState();
}

void AHeistsInteractableActorBase::CommitInteract_Implementation(AActor* Interactor, EHeistsInteractionActionId ActionId)
{
	if (!HasAuthority())
	{
		return;
	}

	ApplyInteractionResult(Interactor, ActionId);
	ClearInteractionState();
}

const FHeistsInteractionAction* AHeistsInteractableActorBase::FindAction(EHeistsInteractionActionId ActionId) const
{
	return Actions.FindByPredicate(
		[ActionId](const FHeistsInteractionAction& Action)
		{
			return Action.ActionId == ActionId;
		});
}

void AHeistsInteractableActorBase::ApplyInteractionResult(AActor* Interactor, EHeistsInteractionActionId ActionId)
{
	UE_LOG(LogTemp, Verbose, TEXT("%s applied interaction action %d"), *GetName(), static_cast<int32>(ActionId));
}

void AHeistsInteractableActorBase::AdvanceInteractionProgress()
{
	if (!HasAuthority() || ActiveInteractionDuration <= 0.f)
	{
		ClearInteractionState();
		return;
	}

	InteractionProgress = FMath::Clamp(InteractionProgress + (0.1f / ActiveInteractionDuration), 0.f, 1.f);
	BP_OnProgressChanged();

	if (InteractionProgress >= 1.f)
	{
		CommitInteract_Implementation(ActiveInteractor, ActiveActionId);
	}
}

void AHeistsInteractableActorBase::ClearInteractionState()
{
	GetWorldTimerManager().ClearTimer(InteractionProgressTimerHandle);
	ActiveInteractor = nullptr;
	ActiveActionId = EHeistsInteractionActionId::None;
	ActiveInteractionDuration = 0.f;
	InteractionProgress = 0.f;
	BP_OnBusyStateChanged();
	BP_OnProgressChanged();
}

void AHeistsInteractableActorBase::ApplyLocalFocusVisualState()
{
	if (!DebugMesh)
	{
		return;
	}

	DebugMesh->SetRenderCustomDepth(bIsLocallyFocused);
	DebugMesh->SetCustomDepthStencilValue(bIsLocallyFocused ? 2 : 0);
	DebugMesh->SetVectorParameterValueOnMaterials(
		TEXT("Color"),
		bIsLocallyFocused ? FVector(0.f, 1.f, 0.f) : FVector(1.f, 1.f, 1.f));
}

void AHeistsInteractableActorBase::OnRep_ActiveInteractor()
{
	BP_OnBusyStateChanged();
}

void AHeistsInteractableActorBase::OnRep_Progress()
{
	BP_OnProgressChanged();
}
