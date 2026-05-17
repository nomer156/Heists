// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HeistsGameMode.generated.h"

/**
 * AHeistsGameMode
 *
 * Серверный Game Mode. Управляет: фазами миссии, слотами игроков, победой/поражением.
 * Поддерживает: 1-4 игрока, разные конфигурации команд (solo, 4-player, future: 4v4).
 * Работает ТОЛЬКО на сервере (GameMode не реплицируется).
 */
UENUM(BlueprintType)
enum class EHeistPhase : uint8
{
	Lobby       UMETA(DisplayName = "Lobby"),
	Planning    UMETA(DisplayName = "Planning"),
	Heist       UMETA(DisplayName = "Heist"),
	Extraction  UMETA(DisplayName = "Extraction"),
	PostHeist   UMETA(DisplayName = "Post-Heist"),
};

UCLASS()
class HEISTS_API AHeistsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHeistsGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	// --- Фазы миссии ---

	UFUNCTION(BlueprintCallable, Category = "Heists|GameMode")
	void SetHeistPhase(EHeistPhase NewPhase);

	UFUNCTION(BlueprintCallable, Category = "Heists|GameMode")
	EHeistPhase GetHeistPhase() const { return CurrentPhase; }

	// --- Конфигурация команды ---

	// Максимальное число игроков (1-4, или больше для будущих режимов)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|GameMode")
	int32 MaxPlayers = 4;

	// Минимум для старта миссии (solo = 1)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|GameMode")
	int32 MinPlayersToStart = 1;

	// --- Победа / Поражение ---

	UFUNCTION(BlueprintCallable, Category = "Heists|GameMode")
	void TriggerMissionSuccess();

	UFUNCTION(BlueprintCallable, Category = "Heists|GameMode")
	void TriggerMissionFail(const FString& Reason);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Heists|GameMode")
	EHeistPhase CurrentPhase;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Phase1")
	bool bSpawnPhase1PrototypeActors = true;

	void SpawnPhase1PrototypeActors();

	// Blueprint-события для реакции на смену фазы
	UFUNCTION(BlueprintImplementableEvent, Category = "Heists|GameMode")
	void OnHeistPhaseChanged(EHeistPhase NewPhase);

	UFUNCTION(BlueprintImplementableEvent, Category = "Heists|GameMode")
	void OnMissionSuccess();

	UFUNCTION(BlueprintImplementableEvent, Category = "Heists|GameMode")
	void OnMissionFail(const FString& Reason);
};
