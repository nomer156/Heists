// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "Roles/HeistsRoleTypes.h"
#include "HeistsPlayerState.generated.h"

class UAbilitySystemComponent;
class UHeistsAttributeSet;

/**
 * AHeistsPlayerState
 *
 * Состояние игрока — реплицируется на все клиенты.
 * Содержит: имя, роль, очки, статус готовности, GAS owner state.
 */
UCLASS()
class HEISTS_API AHeistsPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AHeistsPlayerState();

	// ~IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Heists|GAS")
	UHeistsAttributeSet* GetHeistsAttributeSet() const { return AttributeSet; }

	// --- Роль игрока ---

	UFUNCTION(BlueprintCallable, Category = "Heists|PlayerState")
	FName GetPlayerRole() const { return PlayerRole; }

	UFUNCTION(BlueprintCallable, Category = "Heists|PlayerState")
	void SetPlayerRole(FName NewRole);

	UFUNCTION(BlueprintCallable, Category = "Heists|Roles")
	EHeistsCrewRole GetCrewRole() const { return CrewRole; }

	UFUNCTION(BlueprintCallable, Category = "Heists|Roles")
	void SetCrewRole(EHeistsCrewRole NewRole);

	// --- Очки (CP — Contribution Points) ---

	UFUNCTION(BlueprintCallable, Category = "Heists|PlayerState")
	int32 GetContributionPoints() const { return ContributionPoints; }

	void AddContributionPoints(int32 Amount);

	// --- Готовность (для лобби) ---

	UFUNCTION(BlueprintCallable, Category = "Heists|PlayerState")
	bool IsReady() const { return bIsReady; }

	UFUNCTION(Server, Reliable)
	void Server_SetReady(bool bReady);

	// --- Статус ---

	UFUNCTION(BlueprintCallable, Category = "Heists|PlayerState")
	bool IsPlayerAlive() const { return bIsPlayerAlive; }

	void SetPlayerAlive(bool bAlive);

protected:
	// GAS lives on PlayerState so abilities and attributes survive pawn possession changes.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Heists|GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Heists|GAS")
	TObjectPtr<UHeistsAttributeSet> AttributeSet;

	// Роль (Coordinator, Breaker, Hacker, Scout, Driver)
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|PlayerState")
	FName PlayerRole;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CrewRole, Category = "Heists|Roles")
	EHeistsCrewRole CrewRole;

	// Очки вклада за сессию
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|PlayerState")
	int32 ContributionPoints;

	// Готов к старту миссии
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsReady, Category = "Heists|PlayerState")
	bool bIsReady;

	// Жив ли игрок
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|PlayerState")
	bool bIsPlayerAlive;

	UFUNCTION()
	void OnRep_IsReady();

	UFUNCTION()
	void OnRep_CrewRole();

	UFUNCTION(BlueprintImplementableEvent, Category = "Heists|PlayerState")
	void BP_OnReadyStateChanged(bool bReady);

	UFUNCTION(BlueprintImplementableEvent, Category = "Heists|Roles")
	void BP_OnCrewRoleChanged(EHeistsCrewRole NewRole);
};
