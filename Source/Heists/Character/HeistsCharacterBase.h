// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "HeistsCharacterBase.generated.h"

class UAbilitySystemComponent;
class UHeistsAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class USpringArmComponent;
class UCameraComponent;

/**
 * AHeistsCharacterBase
 *
 * Базовый класс всех персонажей игры Heists.
 * Содержит: GAS (AbilitySystem), базовые атрибуты, репликацию, движение.
 * От него наследуются AHeistsRobber и AHeistsDriver.
 *
 * Multiplayer: Все свойства помечены Replicated где необходимо.
 * Blueprint использует только для визуала (меш, анимации, звук).
 */
UCLASS(Abstract, Blueprintable)
class HEISTS_API AHeistsCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AHeistsCharacterBase();

	// ~IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Вызывается после репликации всех свойств
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	// --- GAS ---

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Heists|GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Heists|GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHeistsAttributeSet> AttributeSet;

	// Стартовые способности (назначаются в BP дочернего класса)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	// Стартовые эффекты (инициализация атрибутов)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|GAS|Effects")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

	// Инициализация GAS на сервере и клиенте
	void InitializeGAS();
	void ApplyDefaultEffects();
	void GrantDefaultAbilities();

	bool bDefaultEffectsApplied;
	bool bDefaultAbilitiesGranted;

	// --- Состояние персонажа (реплицированные) ---

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|State")
	bool bIsAlive;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|State")
	bool bIsCarryingLoot;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|State")
	float CarryWeight;

	// --- Камера (override в BP дочернего класса) ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Heists|Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Heists|Camera")
	TObjectPtr<UCameraComponent> Camera;

public:
	// --- Публичные геттеры ---

	UFUNCTION(BlueprintCallable, Category = "Heists|State")
	bool IsAlive() const { return bIsAlive; }

	UFUNCTION(BlueprintCallable, Category = "Heists|State")
	bool IsCarryingLoot() const { return bIsCarryingLoot; }

	UFUNCTION(BlueprintCallable, Category = "Heists|State")
	float GetCarryWeight() const { return CarryWeight; }

	UFUNCTION(BlueprintCallable, Category = "Heists|GAS")
	UHeistsAttributeSet* GetHeistsAttributeSet() const { return AttributeSet; }

	// Смерть (Server-authoritative)
	UFUNCTION(BlueprintCallable, Category = "Heists|State")
	virtual void Die();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath();

protected:
	// Оверрайд в дочерних классах для роль-специфичной логики
	virtual void InitializeRole() {}
};
