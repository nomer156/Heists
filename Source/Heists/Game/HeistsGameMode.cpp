// Copyright 2026 Heists. All Rights Reserved.

#include "Game/HeistsGameMode.h"
#include "Character/HeistsRobber.h"
#include "Game/HeistsGameState.h"
#include "Player/HeistsPlayerController.h"
#include "Player/HeistsPlayerState.h"
#include "UI/HeistsHUD.h"

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
