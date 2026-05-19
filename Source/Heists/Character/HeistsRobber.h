// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/HeistsCharacterBase.h"
#include "Roles/HeistsRoleTypes.h"
#include "HeistsRobber.generated.h"

class UHeistsInteractionComponent;
class UHeistsCoverComponent;
class AHeistsLootBag;

/**
 * AHeistsRobber
 *
 * Базовый C++ класс для ВСЕХ ролей грабителей (Coordinator, Breaker, Hacker, Scout).
 * Содержит: общую логику грабителя — взаимодействие с объектами, ношение лута.
 * От него наследуются Blueprint-классы конкретных ролей.
 *
 * Роль-специфичная логика (взлом, взрыв и т.д.) реализуется через GAS Abilities.
 */
UCLASS(Blueprintable)
class HEISTS_API AHeistsRobber : public AHeistsCharacterBase
{
	GENERATED_BODY()

public:
	AHeistsRobber();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Взаимодействие ---

	// Попытка взаимодействия с объектом (Server)
	UFUNCTION(BlueprintCallable, Category = "Heists|Robber|Interaction")
	void TryInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Interact();

	UFUNCTION(BlueprintPure, Category = "Heists|Robber|Interaction")
	UHeistsInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

	UFUNCTION(BlueprintPure, Category = "Heists|Robber|Cover")
	UHeistsCoverComponent* GetCoverComponent() const { return CoverComponent; }

	// --- Лут ---

	// Подобрать сумку с лутом (Server)
	UFUNCTION(BlueprintCallable, Category = "Heists|Robber|Loot")
	void PickUpLoot(float Weight);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_PickUpLoot(float Weight);

	// Бросить сумку (Server)
	UFUNCTION(BlueprintCallable, Category = "Heists|Robber|Loot")
	void DropLoot();

	UFUNCTION(Server, Reliable)
	void Server_DropLoot();

	UFUNCTION(BlueprintCallable, Category = "Heists|Robber|Loot")
	bool CanCarryLootBag() const;

	UFUNCTION(BlueprintCallable, Category = "Heists|Robber|Loot")
	bool SetCarriedLootBag(AHeistsLootBag* LootBag);

	UFUNCTION(BlueprintCallable, Category = "Heists|Robber|Loot")
	int32 DepositCarriedLoot();

	UFUNCTION(BlueprintPure, Category = "Heists|Robber|Loot")
	AHeistsLootBag* GetCarriedLootBag() const { return CarriedLootBag; }

	// --- Состояние роли ---

	// Тип роли (Coordinator / Breaker / Hacker / Scout) — задаётся в BP
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Robber|Role", Replicated)
	FName RoleType;

	UFUNCTION(BlueprintPure, Category = "Heists|Robber|Role")
	EHeistsCrewRole GetCrewRole() const;

	UFUNCTION(BlueprintPure, Category = "Heists|Robber|Role")
	FHeistsRoleTuning GetRoleTuning() const;

protected:
	virtual void InitializeRole() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Heists|Robber|Interaction")
	TObjectPtr<UHeistsInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Heists|Robber|Cover")
	TObjectPtr<UHeistsCoverComponent> CoverComponent;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CarriedLootBag, Category = "Heists|Robber|Loot")
	TObjectPtr<AHeistsLootBag> CarriedLootBag;

	float BaseWalkSpeed = 400.f;

	// Радиус взаимодействия с объектами
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Robber|Interaction")
	float InteractionRadius = 150.f;

	void ApplyLootCarryMovement();
	void ClearCarriedLootBag();

	UFUNCTION()
	void OnRep_CarriedLootBag();
};
