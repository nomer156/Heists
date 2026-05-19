// Copyright 2026 Heists. All Rights Reserved.

#include "Character/HeistsRobber.h"
#include "Character/HeistsCoverComponent.h"
#include "Interaction/HeistsInteractionComponent.h"
#include "Loot/HeistsLootBag.h"
#include "Player/HeistsPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"


AHeistsRobber::AHeistsRobber()
{
	RoleType = NAME_None;
	InteractionComponent = CreateDefaultSubobject<UHeistsInteractionComponent>(TEXT("InteractionComponent"));
	CoverComponent = CreateDefaultSubobject<UHeistsCoverComponent>(TEXT("CoverComponent"));
}

void AHeistsRobber::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHeistsRobber, RoleType);
	DOREPLIFETIME(AHeistsRobber, CarriedLootBag);
}

void AHeistsRobber::InitializeRole()
{
	// Базовая инициализация роли — расширяется в BP через DefaultAbilities/DefaultEffects
	UE_LOG(LogTemp, Log, TEXT("AHeistsRobber::InitializeRole — Role: %s"), *RoleType.ToString());
}

EHeistsCrewRole AHeistsRobber::GetCrewRole() const
{
	if (const AHeistsPlayerState* HeistsPlayerState = GetPlayerState<AHeistsPlayerState>())
	{
		if (HeistsPlayerState->GetCrewRole() != EHeistsCrewRole::None)
		{
			return HeistsPlayerState->GetCrewRole();
		}
	}

	if (RoleType == TEXT("Coordinator"))
	{
		return EHeistsCrewRole::Coordinator;
	}
	if (RoleType == TEXT("Hacker"))
	{
		return EHeistsCrewRole::Hacker;
	}
	if (RoleType == TEXT("Breaker"))
	{
		return EHeistsCrewRole::Breaker;
	}
	if (RoleType == TEXT("Scout"))
	{
		return EHeistsCrewRole::Scout;
	}
	if (RoleType == TEXT("Driver"))
	{
		return EHeistsCrewRole::Driver;
	}

	return EHeistsCrewRole::None;
}

FHeistsRoleTuning AHeistsRobber::GetRoleTuning() const
{
	FHeistsRoleTuning Tuning;
	Tuning.Role = GetCrewRole();

	switch (Tuning.Role)
	{
	case EHeistsCrewRole::Coordinator:
		Tuning.TeamInfoRangeBonus = 400.f;
		break;
	case EHeistsCrewRole::Hacker:
		Tuning.HackDurationMultiplier = 0.65f;
		break;
	case EHeistsCrewRole::Breaker:
		Tuning.ForceDurationMultiplier = 0.65f;
		break;
	case EHeistsCrewRole::Scout:
		Tuning.QuietInteractionMultiplier = 0.85f;
		Tuning.ContextScanRangeBonus = 150.f;
		break;
	default:
		break;
	}

	return Tuning;
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
	ApplyLootCarryMovement();

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

	if (CarriedLootBag)
	{
		const FVector DropLocation = GetActorLocation() + GetActorForwardVector() * 100.f;
		AHeistsLootBag* LootBagToDrop = CarriedLootBag;
		ClearCarriedLootBag();
		LootBagToDrop->DropFromCarrier(DropLocation);
		return;
	}

	ClearCarriedLootBag();
	UE_LOG(LogTemp, Log, TEXT("Server_DropLoot: loot dropped"));
}

bool AHeistsRobber::CanCarryLootBag() const
{
	return !CarriedLootBag && !bIsCarryingLoot;
}

bool AHeistsRobber::SetCarriedLootBag(AHeistsLootBag* LootBag)
{
	if (!HasAuthority() || !LootBag || !CanCarryLootBag())
	{
		return false;
	}

	CarriedLootBag = LootBag;
	bIsCarryingLoot = true;
	CarryWeight = LootBag->GetWeight();
	ApplyLootCarryMovement();
	return true;
}

int32 AHeistsRobber::DepositCarriedLoot()
{
	if (!HasAuthority() || !CarriedLootBag)
	{
		return 0;
	}

	AHeistsLootBag* LootBagToDeposit = CarriedLootBag;
	const int32 LootValue = LootBagToDeposit->GetLootValue();
	ClearCarriedLootBag();
	LootBagToDeposit->Destroy();
	return LootValue;
}

void AHeistsRobber::ApplyLootCarryMovement()
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		if (BaseWalkSpeed <= 0.f)
		{
			BaseWalkSpeed = MovementComponent->MaxWalkSpeed;
		}

		MovementComponent->MaxWalkSpeed = bIsCarryingLoot ? BaseWalkSpeed * 0.75f : BaseWalkSpeed;
	}
}

void AHeistsRobber::ClearCarriedLootBag()
{
	CarriedLootBag = nullptr;
	bIsCarryingLoot = false;
	CarryWeight = 0.f;
	ApplyLootCarryMovement();
}

void AHeistsRobber::OnRep_CarriedLootBag()
{
	bIsCarryingLoot = CarriedLootBag != nullptr;
	CarryWeight = CarriedLootBag ? CarriedLootBag->GetWeight() : 0.f;
	ApplyLootCarryMovement();
}
