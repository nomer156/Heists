// Copyright 2026 Heists. All Rights Reserved.

#include "Player/HeistsPlayerState.h"
#include "AbilitySystem/HeistsAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

AHeistsPlayerState::AHeistsPlayerState()
{
	SetNetUpdateFrequency(100.f);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UHeistsAttributeSet>(TEXT("AttributeSet"));
	AbilitySystemComponent->AddAttributeSetSubobject(AttributeSet.Get());

	PlayerRole = NAME_None;
	CrewRole = EHeistsCrewRole::None;
	ContributionPoints = 0;
	bIsReady = false;
	bIsPlayerAlive = true;
}

UAbilitySystemComponent* AHeistsPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AHeistsPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHeistsPlayerState, PlayerRole);
	DOREPLIFETIME(AHeistsPlayerState, CrewRole);
	DOREPLIFETIME(AHeistsPlayerState, ContributionPoints);
	DOREPLIFETIME(AHeistsPlayerState, bIsReady);
	DOREPLIFETIME(AHeistsPlayerState, bIsPlayerAlive);
}

void AHeistsPlayerState::SetPlayerRole(FName NewRole)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		PlayerRole = NewRole;
	}
}

void AHeistsPlayerState::SetCrewRole(EHeistsCrewRole NewRole)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	CrewRole = NewRole;
	if (const UEnum* CrewRoleEnum = StaticEnum<EHeistsCrewRole>())
	{
		PlayerRole = FName(*CrewRoleEnum->GetNameStringByValue(static_cast<int64>(CrewRole)));
	}
	BP_OnCrewRoleChanged(CrewRole);
}

void AHeistsPlayerState::AddContributionPoints(int32 Amount)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		ContributionPoints += Amount;
	}
}

void AHeistsPlayerState::Server_SetReady_Implementation(bool bReady)
{
	bIsReady = bReady;
	// OnRep вызовется автоматически на клиентах
}

void AHeistsPlayerState::SetPlayerAlive(bool bAlive)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bIsPlayerAlive = bAlive;
	}
}

void AHeistsPlayerState::OnRep_IsReady()
{
	BP_OnReadyStateChanged(bIsReady);
}

void AHeistsPlayerState::OnRep_CrewRole()
{
	BP_OnCrewRoleChanged(CrewRole);
}
