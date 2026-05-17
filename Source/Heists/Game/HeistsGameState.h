// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Game/HeistsGameMode.h"
#include "GameplayTagContainer.h"
#include "HeistsGameState.generated.h"

/**
 * AHeistsGameState
 *
 * Реплицируется на все клиенты. Содержит публичное состояние сессии:
 * текущая фаза, таймеры, уровень тревоги, количество собранного лута.
 */
UCLASS()
class HEISTS_API AHeistsGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AHeistsGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Вызывается GameMode на сервере для смены фазы
	void SetCurrentPhase(EHeistPhase NewPhase);

	// --- Геттеры для BP ---

	UFUNCTION(BlueprintCallable, Category = "Heists|GameState")
	EHeistPhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintCallable, Category = "Heists|GameState")
	int32 GetAlertLevel() const { return AlertLevel; }

	UFUNCTION(BlueprintCallable, Category = "Heists|GameState")
	int32 GetLootCollected() const { return LootCollected; }

	UFUNCTION(BlueprintCallable, Category = "Heists|GameState")
	float GetMissionTimer() const { return MissionTimer; }

	UFUNCTION(BlueprintCallable, Category = "Heists|SharedItems")
	bool HasSharedCrewItem(FGameplayTag ItemTag) const;

	UFUNCTION(BlueprintCallable, Category = "Heists|SharedItems")
	void AddSharedCrewItem(FGameplayTag ItemTag);

	UFUNCTION(BlueprintCallable, Category = "Heists|SharedItems")
	FGameplayTagContainer GetSharedCrewItems() const { return SharedCrewItems; }

	// Добавить лут (Server only)
	void AddLoot(int32 Amount);

	// Изменить уровень тревоги (Server only, 0-5)
	void SetAlertLevel(int32 NewLevel);

protected:
	// Фаза миссии (реплицируется)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentPhase, Category = "Heists|GameState")
	EHeistPhase CurrentPhase;

	// Уровень тревоги охраны: 0=Спокойно, 1=Подозрение, 2=Локальная тревога, 3=Тревога, 4=Штурм, 5=Полный штурм
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AlertLevel, Category = "Heists|GameState")
	int32 AlertLevel;

	// Собранный лут (в условных единицах)
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|GameState")
	int32 LootCollected;

	// Таймер миссии (секунды)
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|GameState")
	float MissionTimer;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SharedCrewItems, Category = "Heists|SharedItems")
	FGameplayTagContainer SharedCrewItems;

	UFUNCTION()
	void OnRep_CurrentPhase();

	UFUNCTION()
	void OnRep_AlertLevel();

	UFUNCTION()
	void OnRep_SharedCrewItems();

	// Blueprint события
	UFUNCTION(BlueprintImplementableEvent, Category = "Heists|GameState")
	void BP_OnPhaseChanged(EHeistPhase NewPhase);

	UFUNCTION(BlueprintImplementableEvent, Category = "Heists|GameState")
	void BP_OnAlertLevelChanged(int32 NewLevel);

	UFUNCTION(BlueprintImplementableEvent, Category = "Heists|SharedItems")
	void BP_OnSharedCrewItemsChanged();
};
