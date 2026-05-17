// Copyright 2026 Heists. All Rights Reserved.

#include "Game/HeistsGameMode.h"
#include "Character/HeistsRobber.h"
#include "Game/HeistsGameState.h"
#include "Interaction/HeistsDoorActor.h"
#include "Interaction/HeistsPickupActor.h"
#include "Interaction/HeistsTerminalActor.h"
#include "Loot/HeistsExtractionZone.h"
#include "Loot/HeistsLootContainer.h"
#include "Player/HeistsPlayerController.h"
#include "Player/HeistsPlayerState.h"
#include "UI/HeistsHUD.h"
#include "Engine/World.h"

AHeistsGameMode::AHeistsGameMode()
{
	CurrentPhase = EHeistPhase::Lobby;

	// Дефолтные классы — переопределяются в BP_HeistsGameMode
	PlayerControllerClass = AHeistsPlayerController::StaticClass();
	PlayerStateClass = AHeistsPlayerState::StaticClass();
	GameStateClass = AHeistsGameState::StaticClass();
	DefaultPawnClass = AHeistsRobber::StaticClass();
	HUDClass = AHeistsHUD::StaticClass();
}

void AHeistsGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("HeistsGameMode: BeginPlay, Phase=%d"), (int32)CurrentPhase);

	if (bSpawnPhase1PrototypeActors)
	{
		SpawnPhase1PrototypeActors();
	}
}

void AHeistsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	UE_LOG(LogTemp, Log, TEXT("HeistsGameMode: Player logged in: %s"), *GetNameSafe(NewPlayer));
}

void AHeistsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	UE_LOG(LogTemp, Log, TEXT("HeistsGameMode: Player left: %s"), *GetNameSafe(Exiting));
}

void AHeistsGameMode::SetHeistPhase(EHeistPhase NewPhase)
{
	if (CurrentPhase == NewPhase) return;

	CurrentPhase = NewPhase;
	UE_LOG(LogTemp, Log, TEXT("HeistsGameMode: Phase changed to %d"), (int32)CurrentPhase);
	OnHeistPhaseChanged(NewPhase);

	// Синхронизируем фазу в GameState (для клиентов)
	if (AHeistsGameState* GS = GetGameState<AHeistsGameState>())
	{
		GS->SetCurrentPhase(NewPhase);
	}
}

void AHeistsGameMode::TriggerMissionSuccess()
{
	SetHeistPhase(EHeistPhase::PostHeist);
	OnMissionSuccess();
	UE_LOG(LogTemp, Log, TEXT("HeistsGameMode: MISSION SUCCESS"));
}

void AHeistsGameMode::TriggerMissionFail(const FString& Reason)
{
	SetHeistPhase(EHeistPhase::PostHeist);
	OnMissionFail(Reason);
	UE_LOG(LogTemp, Log, TEXT("HeistsGameMode: MISSION FAIL — %s"), *Reason);
}

void AHeistsGameMode::SpawnPhase1PrototypeActors()
{
	UWorld* World = GetWorld();
	if (!World || !HasAuthority())
	{
		return;
	}

	const FString MapName = World->GetMapName();
	if (!MapName.Contains(TEXT("MainMap")))
	{
		return;
	}

	auto SpawnPrototypeActor = [World](TSubclassOf<AActor> ActorClass, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParameters);
		if (SpawnedActor)
		{
			SpawnedActor->SetActorScale3D(Scale);
		}
		return SpawnedActor;
	};

	SpawnPrototypeActor(AHeistsDoorActor::StaticClass(), FVector(250.f, 0.f, 60.f), FRotator::ZeroRotator, FVector(0.7f, 0.12f, 1.4f));
	SpawnPrototypeActor(AHeistsTerminalActor::StaticClass(), FVector(500.f, 180.f, 55.f), FRotator::ZeroRotator, FVector(0.45f, 0.45f, 0.8f));
	SpawnPrototypeActor(AHeistsPickupActor::StaticClass(), FVector(120.f, 220.f, 45.f), FRotator::ZeroRotator, FVector(0.25f, 0.25f, 0.08f));
	SpawnPrototypeActor(AHeistsLootContainer::StaticClass(), FVector(650.f, -160.f, 55.f), FRotator::ZeroRotator, FVector(0.7f, 0.7f, 0.5f));
	SpawnPrototypeActor(AHeistsExtractionZone::StaticClass(), FVector(900.f, 0.f, 25.f), FRotator::ZeroRotator, FVector(1.5f, 1.5f, 0.08f));

	UE_LOG(LogTemp, Log, TEXT("HeistsGameMode: spawned Phase 1 prototype actors on MainMap."));
}
