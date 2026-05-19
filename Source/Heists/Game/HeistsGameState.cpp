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
	DOREPLIFETIME(AHeistsGameState, SharedCrewItems);
	DOREPLIFETIME(AHeistsGameState, PrototypeMissionResult);
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

bool AHeistsGameState::HasSharedCrewItem(FGameplayTag ItemTag) const
{
	return ItemTag.IsValid() && SharedCrewItems.HasTagExact(ItemTag);
}

void AHeistsGameState::AddSharedCrewItem(FGameplayTag ItemTag)
{
	if (!HasAuthority() || !ItemTag.IsValid())
	{
		return;
	}

	if (SharedCrewItems.HasTagExact(ItemTag))
	{
		return;
	}

	SharedCrewItems.AddTag(ItemTag);
	BP_OnSharedCrewItemsChanged();
	UE_LOG(LogTemp, Log, TEXT("HeistsGameState: shared crew item acquired: %s"), *ItemTag.ToString());
}

void AHeistsGameState::StartPrototypeMission()
{
	if (!HasAuthority())
	{
		return;
	}

	PrototypeMissionResult = {};
	PrototypeMissionResult.bMissionActive = true;
	PrototypeMissionResult.SharedItemsAcquired = SharedCrewItems.Num();
	PrototypeMissionResult.DeliveredLootValue = LootCollected;
	BP_OnPrototypeMissionResultChanged(PrototypeMissionResult);
}

void AHeistsGameState::CompletePrototypeMission()
{
	if (!HasAuthority())
	{
		return;
	}

	PrototypeMissionResult.bMissionActive = false;
	PrototypeMissionResult.bMissionCompleted = true;
	PrototypeMissionResult.bMissionFailed = false;
	PrototypeMissionResult.DeliveredLootValue = LootCollected;
	PrototypeMissionResult.SharedItemsAcquired = SharedCrewItems.Num();
	BP_OnPrototypeMissionResultChanged(PrototypeMissionResult);
}

void AHeistsGameState::FailPrototypeMission()
{
	if (!HasAuthority())
	{
		return;
	}

	PrototypeMissionResult.bMissionActive = false;
	PrototypeMissionResult.bMissionCompleted = false;
	PrototypeMissionResult.bMissionFailed = true;
	PrototypeMissionResult.DeliveredLootValue = LootCollected;
	PrototypeMissionResult.SharedItemsAcquired = SharedCrewItems.Num();
	BP_OnPrototypeMissionResultChanged(PrototypeMissionResult);
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

void AHeistsGameState::OnRep_SharedCrewItems()
{
	BP_OnSharedCrewItemsChanged();
}

void AHeistsGameState::OnRep_PrototypeMissionResult()
{
	BP_OnPrototypeMissionResultChanged(PrototypeMissionResult);
}
