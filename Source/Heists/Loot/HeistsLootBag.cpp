// Copyright 2026 Heists. All Rights Reserved.

#include "Loot/HeistsLootBag.h"

#include "Character/HeistsRobber.h"
#include "Net/UnrealNetwork.h"

AHeistsLootBag::AHeistsLootBag()
{
	Actions = {
		{ EHeistsInteractionActionId::Pickup, FText::FromString(TEXT("Pickup")), EHeistsInteractionTaskType::Instant, EHeistsInteractionColor::Green },
		{ EHeistsInteractionActionId::Drop, FText::FromString(TEXT("Drop")), EHeistsInteractionTaskType::Instant, EHeistsInteractionColor::Gray }
	};
}

void AHeistsLootBag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHeistsLootBag, LootValue);
	DOREPLIFETIME(AHeistsLootBag, Weight);
	DOREPLIFETIME(AHeistsLootBag, bIsCarried);
	DOREPLIFETIME(AHeistsLootBag, Carrier);
}

bool AHeistsLootBag::TryPickup(AHeistsRobber* Robber)
{
	if (!HasAuthority() || !Robber || bIsCarried)
	{
		return false;
	}

	if (!Robber->SetCarriedLootBag(this))
	{
		return false;
	}

	Carrier = Robber;
	bIsCarried = true;
	SetOwner(Robber);
	AttachToActor(Robber, FAttachmentTransformRules::KeepWorldTransform);
	ApplyCarryVisualState();
	return true;
}

void AHeistsLootBag::DropFromCarrier(const FVector& DropLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetOwner(nullptr);
	Carrier = nullptr;
	bIsCarried = false;
	SetActorLocation(DropLocation);
	ApplyCarryVisualState();
}

void AHeistsLootBag::ApplyInteractionResult(AActor* Interactor, EHeistsInteractionActionId ActionId)
{
	AHeistsRobber* Robber = Cast<AHeistsRobber>(Interactor);
	if (!Robber)
	{
		return;
	}

	if (ActionId == EHeistsInteractionActionId::Pickup)
	{
		TryPickup(Robber);
	}
	else if (ActionId == EHeistsInteractionActionId::Drop && Carrier == Robber)
	{
		Robber->DropLoot();
	}
}

void AHeistsLootBag::OnRep_CarryState()
{
	ApplyCarryVisualState();
}

void AHeistsLootBag::ApplyCarryVisualState()
{
	SetActorHiddenInGame(bIsCarried);
	SetActorEnableCollision(!bIsCarried);
}
