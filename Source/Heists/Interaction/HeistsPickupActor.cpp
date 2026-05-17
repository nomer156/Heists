// Copyright 2026 Heists. All Rights Reserved.

#include "Interaction/HeistsPickupActor.h"

#include "Game/HeistsGameState.h"

AHeistsPickupActor::AHeistsPickupActor()
{
	SharedItemTag = FGameplayTag::RequestGameplayTag(TEXT("Heists.SharedItem.Keycard.Blue"), false);

	Actions = {
		{ EHeistsInteractionActionId::Pickup, FText::FromString(TEXT("Pickup")), EHeistsInteractionTaskType::Instant, EHeistsInteractionColor::Blue }
	};
}

void AHeistsPickupActor::ApplyInteractionResult(AActor* Interactor, EHeistsInteractionActionId ActionId)
{
	if (ActionId != EHeistsInteractionActionId::Pickup)
	{
		return;
	}

	if (SharedItemTag.IsValid())
	{
		if (AHeistsGameState* HeistsGameState = GetWorld()->GetGameState<AHeistsGameState>())
		{
			HeistsGameState->AddSharedCrewItem(SharedItemTag);
		}
	}

	if (bDestroyOnPickup)
	{
		Destroy();
	}
	else
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
}
