// Copyright 2026 Heists. All Rights Reserved.

#include "Character/HeistsRobber.h"
#include "Interaction/HeistsInteractionComponent.h"
#include "Net/UnrealNetwork.h"


AHeistsRobber::AHeistsRobber()
{
	RoleType = NAME_None;
	InteractionComponent = CreateDefaultSubobject<UHeistsInteractionComponent>(TEXT("InteractionComponent"));
}

void AHeistsRobber::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHeistsRobber, RoleType);
}

void AHeistsRobber::InitializeRole()
{
	// Базовая инициализация роли — расширяется в BP через DefaultAbilities/DefaultEffects
	UE_LOG(LogTemp, Log, TEXT("AHeistsRobber::InitializeRole — Role: %s"), *RoleType.ToString());
}

void AHeistsRobber::TryInteract()
{
	if (!HasAuthority())
	{
		Server_Interact();
		return;
	}
	Server_Interact_Implementation();
}

bool AHeistsRobber::Server_Interact_Validate()
{
	return true;
}

void AHeistsRobber::Server_Interact_Implementation()
{
	if (!HasAuthority()) return;

	if (InteractionComponent)
	{
		InteractionComponent->RequestPrimaryInteraction();
	}
}

void AHeistsRobber::PickUpLoot(float Weight)
{
	if (!HasAuthority())
	{
		Server_PickUpLoot(Weight);
		return;
	}
	Server_PickUpLoot_Implementation(Weight);
}

bool AHeistsRobber::Server_PickUpLoot_Validate(float Weight)
{
	return Weight >= 0.f && Weight <= 100.f;
}

void AHeistsRobber::Server_PickUpLoot_Implementation(float Weight)
{
	if (!HasAuthority()) return;

	bIsCarryingLoot = true;
	CarryWeight = Weight;

	// TODO: GAS — применить эффект замедления от веса
	UE_LOG(LogTemp, Log, TEXT("Server_PickUpLoot: Weight=%.1f"), Weight);
}

void AHeistsRobber::DropLoot()
{
	if (!HasAuthority())
	{
		Server_DropLoot();
		return;
	}
	Server_DropLoot_Implementation();
}

void AHeistsRobber::Server_DropLoot_Implementation()
{
	if (!HasAuthority()) return;

	bIsCarryingLoot = false;
	CarryWeight = 0.f;

	// TODO: Заспавнить физический актор сумки в мире
	UE_LOG(LogTemp, Log, TEXT("Server_DropLoot: loot dropped"));
}
