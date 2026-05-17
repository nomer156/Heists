// Copyright 2026 Heists. All Rights Reserved.

#include "AbilitySystem/HeistsAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Character/HeistsCharacterBase.h"

UHeistsAttributeSet::UHeistsAttributeSet()
{
	// Значения по умолчанию (переопределяются через GE_InitAttributes)
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitStamina(100.f);
	InitMaxStamina(100.f);
	InitMoveSpeed(400.f);
	InitNoiseLevel(0.f);
}

void UHeistsAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHeistsAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHeistsAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHeistsAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHeistsAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHeistsAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHeistsAttributeSet, NoiseLevel, COND_None, REPNOTIFY_Always);
}

void UHeistsAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Клампинг значений до применения
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
	else if (Attribute == GetNoiseLevelAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 100.f);
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

void UHeistsAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Зажимаем Health между 0 и MaxHealth
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));

		// Если здоровье == 0, вызываем смерть
		if (GetHealth() <= 0.f)
		{
			AHeistsCharacterBase* OwnerChar = Cast<AHeistsCharacterBase>(Data.Target.GetAvatarActor());
			if (OwnerChar && OwnerChar->IsAlive())
			{
				OwnerChar->Die();
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));
	}
}

void UHeistsAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHeistsAttributeSet, Health, OldHealth);
}

void UHeistsAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHeistsAttributeSet, MaxHealth, OldMaxHealth);
}

void UHeistsAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHeistsAttributeSet, Stamina, OldStamina);
}

void UHeistsAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHeistsAttributeSet, MaxStamina, OldMaxStamina);
}

void UHeistsAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHeistsAttributeSet, MoveSpeed, OldMoveSpeed);
}

void UHeistsAttributeSet::OnRep_NoiseLevel(const FGameplayAttributeData& OldNoiseLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHeistsAttributeSet, NoiseLevel, OldNoiseLevel);
}
