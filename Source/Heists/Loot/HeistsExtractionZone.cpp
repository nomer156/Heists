// Copyright 2026 Heists. All Rights Reserved.

#include "Loot/HeistsExtractionZone.h"

#include "Character/HeistsRobber.h"
#include "Game/HeistsGameState.h"

AHeistsExtractionZone::AHeistsExtractionZone()
{
	Actions = {
		{ EHeistsInteractionActionId::Deposit, FText::FromString(TEXT("Deposit")), EHeistsInteractionTaskType::Instant, EHeistsInteractionColor::Blue }
	};
}

void AHeistsExtractionZone::ApplyInteractionResult(AActor* Interactor, EHeistsInteractionActionId ActionId)
{
	if (ActionId != EHeistsInteractionActionId::Deposit)
	{
		return;
	}

	AHeistsRobber* Robber = Cast<AHeistsRobber>(Interactor);
	if (!Robber)
	{
		return;
	}

	const int32 DepositedValue = Robber->DepositCarriedLoot();
	if (DepositedValue <= 0)
	{
		return;
	}

	if (AHeistsGameState* HeistsGameState = GetWorld()->GetGameState<AHeistsGameState>())
	{
		HeistsGameState->AddLoot(DepositedValue);
	}
}
