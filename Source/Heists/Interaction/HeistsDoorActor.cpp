// Copyright 2026 Heists. All Rights Reserved.

#include "Interaction/HeistsDoorActor.h"

#include "Net/UnrealNetwork.h"

AHeistsDoorActor::AHeistsDoorActor()
{
	Actions = {
		{ EHeistsInteractionActionId::Open, FText::FromString(TEXT("Open")), EHeistsInteractionTaskType::Instant, EHeistsInteractionColor::Green },
		{ EHeistsInteractionActionId::Close, FText::FromString(TEXT("Close")), EHeistsInteractionTaskType::Instant, EHeistsInteractionColor::Green },
		{ EHeistsInteractionActionId::Peek, FText::FromString(TEXT("Peek")), EHeistsInteractionTaskType::Instant, EHeistsInteractionColor::Green },
		{ EHeistsInteractionActionId::Lock, FText::FromString(TEXT("Lock")), EHeistsInteractionTaskType::Instant, EHeistsInteractionColor::Blue },
		{ EHeistsInteractionActionId::Unlock, FText::FromString(TEXT("Unlock")), EHeistsInteractionTaskType::HoldProgress, EHeistsInteractionColor::Blue, 1.0f },
		{ EHeistsInteractionActionId::Breach, FText::FromString(TEXT("Breach")), EHeistsInteractionTaskType::HoldProgress, EHeistsInteractionColor::Red, 1.5f }
	};
}

void AHeistsDoorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHeistsDoorActor, bIsOpen);
	DOREPLIFETIME(AHeistsDoorActor, bIsLocked);
	DOREPLIFETIME(AHeistsDoorActor, bIsPeeked);
}

void AHeistsDoorActor::ApplyInteractionResult(AActor* Interactor, EHeistsInteractionActionId ActionId)
{
	switch (ActionId)
	{
	case EHeistsInteractionActionId::Open:
		if (!bIsLocked)
		{
			bIsOpen = true;
			bIsPeeked = false;
		}
		break;
	case EHeistsInteractionActionId::Close:
		bIsOpen = false;
		bIsPeeked = false;
		break;
	case EHeistsInteractionActionId::Peek:
		if (!bIsLocked)
		{
			bIsOpen = false;
			bIsPeeked = true;
		}
		break;
	case EHeistsInteractionActionId::Lock:
		if (!bIsOpen)
		{
			bIsLocked = true;
		}
		break;
	case EHeistsInteractionActionId::Unlock:
		bIsLocked = false;
		break;
	case EHeistsInteractionActionId::Breach:
		bIsLocked = false;
		bIsOpen = true;
		bIsPeeked = false;
		UE_LOG(LogTemp, Log, TEXT("Door breached: %s"), *GetName());
		break;
	default:
		break;
	}

	BP_OnDoorStateChanged();
}

void AHeistsDoorActor::OnRep_DoorState()
{
	BP_OnDoorStateChanged();
}
