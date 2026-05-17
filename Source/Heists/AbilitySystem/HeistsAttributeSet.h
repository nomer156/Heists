// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "HeistsAttributeSet.generated.h"

// Удобный макрос для объявления атрибутов GAS
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UHeistsAttributeSet
 *
 * Набор атрибутов для всех персонажей Heists.
 * Реплицируется автоматически через GAS.
 */
UCLASS()
class HEISTS_API UHeistsAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UHeistsAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// --- Здоровье ---
	UPROPERTY(BlueprintReadOnly, Category = "Heists|Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UHeistsAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Heists|Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UHeistsAttributeSet, MaxHealth)

	// --- Стамина ---
	UPROPERTY(BlueprintReadOnly, Category = "Heists|Attributes", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UHeistsAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, Category = "Heists|Attributes", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UHeistsAttributeSet, MaxStamina)

	// --- Скорость передвижения (модифицируется GAS) ---
	UPROPERTY(BlueprintReadOnly, Category = "Heists|Attributes", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UHeistsAttributeSet, MoveSpeed)

	// --- Шум (влияет на обнаружение охраной) ---
	UPROPERTY(BlueprintReadOnly, Category = "Heists|Attributes", ReplicatedUsing = OnRep_NoiseLevel)
	FGameplayAttributeData NoiseLevel;
	ATTRIBUTE_ACCESSORS(UHeistsAttributeSet, NoiseLevel)

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);

	UFUNCTION()
	void OnRep_NoiseLevel(const FGameplayAttributeData& OldNoiseLevel);
};
