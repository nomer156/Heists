// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/HeistsCharacterBase.h"
#include "HeistsDriver.generated.h"

/**
 * AHeistsDriver
 *
 * Класс водителя — 4-й игрок команды.
 * Водитель находится на отдельной карте (карта района/улиц).
 * Управляет транспортом, координирует эвакуацию, отвлекает полицию.
 *
 * Роль-специфичная логика: управление авто, маршруты, радиосвязь — через GAS Abilities.
 */
UCLASS(Blueprintable)
class HEISTS_API AHeistsDriver : public AHeistsCharacterBase
{
	GENERATED_BODY()

public:
	AHeistsDriver();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Транспорт ---

	// Войти в машину (Server)
	UFUNCTION(BlueprintCallable, Category = "Heists|Driver|Vehicle")
	void EnterVehicle(AActor* Vehicle);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_EnterVehicle(AActor* Vehicle);

	// Выйти из машины (Server)
	UFUNCTION(BlueprintCallable, Category = "Heists|Driver|Vehicle")
	void ExitVehicle();

	UFUNCTION(Server, Reliable)
	void Server_ExitVehicle();

	// --- Эвакуация ---

	// Сигнал "готов к эвакуации" грабителям
	UFUNCTION(BlueprintCallable, Category = "Heists|Driver|Extraction")
	void SignalEvacReady();

	UFUNCTION(Server, Reliable)
	void Server_SignalEvacReady();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnEvacReady();

	// --- Состояние ---

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|Driver")
	bool bIsInVehicle;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|Driver")
	bool bEvacReady;

protected:
	virtual void InitializeRole() override;
};
