// Copyright 2026 Heists. All Rights Reserved.

#include "Game/HeistsGameState.h"
#include "Net/UnrealNetwork.h"

AHeistsGameState::AHeistsGameState()
{
	CurrentPhase = EHeistPhase::Lobby;
	AlertLevel = 0;
	LootCollected = 0;
	MissionTimer = 0.f;
}

void AHeistsGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHeistsGameState, CurrentPhase);
	DOREPLIFETIME(AHeistsGameState, AlertLevel);
	DOREPLIFETIME(AHeistsGameState, LootCollected);
	DOREPLIFETIME(AHeistsGameState, MissionTimer);
}

void AHeistsGameState::SetCurrentPhase(EHeistPhase NewPhase)
{
	if (!HasAuthority())
	{
		return;
	}

	if (CurrentPhase == NewPhase) return;
	CurrentPhase = NewPhase;
	// OnRep вызовется автоматически на клиентах через репликацию
}

void AHeistsGameState::AddLoot(int32 Amount)
{
	if (!HasAuthority())
	{
		return;
	}

	LootCollected += Amount;
	UE_LOG(LogTemp, Log, TEXT("HeistsGameState: LootCollected=%d"), LootCollected);
}

void AHeistsGameState::SetAlertLevel(int32 NewLevel)
{
	if (!HasAuthority())
	{
		return;
	}

	AlertLevel = FMath::Clamp(NewLevel, 0, 5);
	UE_LOG(LogTemp, Log, TEXT("HeistsGameState: AlertLevel=%d"), AlertLevel);
}

void AHeistsGameState::OnRep_CurrentPhase()
{
	BP_OnPhaseChanged(CurrentPhase);
}

void AHeistsGameState::OnRep_AlertLevel()
{
	BP_OnAlertLevelChanged(AlertLevel);
}
