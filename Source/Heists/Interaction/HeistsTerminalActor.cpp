// Copyright 2026 Heists. All Rights Reserved.

#include "Interaction/HeistsTerminalActor.h"

#include "Net/UnrealNetwork.h"

AHeistsTerminalActor::AHeistsTerminalActor()
{
	FHeistsInteractionAction HackAction;
	HackAction.ActionId = EHeistsInteractionActionId::Hack;
	HackAction.DisplayName = FText::FromString(TEXT("Hack"));
	HackAction.TaskType = EHeistsInteractionTaskType::HoldProgress;
	HackAction.Color = EHeistsInteractionColor::Yellow;
	HackAction.Duration = 3.f;
	HackAction.ProgressBehavior = EHeistsInteractionProgressBehavior::PreserveOnCancel;
	Actions.Add(HackAction);
}

void AHeistsTerminalActor::ApplyInteractionResult(AActor* Interactor, EHeistsInteractionActionId ActionId)
{
	if (ActionId == EHeistsInteractionActionId::Hack)
	{
		bWasCompleted = true;
		UE_LOG(LogTemp, Log, TEXT("Terminal hacked: %s"), *GetName());
	}
}

void AHeistsTerminalActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHeistsTerminalActor, bWasCompleted);
}
