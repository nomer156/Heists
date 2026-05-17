// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HeistsInteractionTypes.generated.h"

UENUM(BlueprintType)
enum class EHeistsInteractionActionId : uint8
{
	None,
	Open,
	Close,
	Peek,
	Lock,
	Unlock,
	Breach,
	Hack,
	Activate,
	ScanFingerprint,
	TimingInput,
	Pickup,
	Drop,
	Deposit,
	Search,
	Disable,
	Inspect
};

UENUM(BlueprintType)
enum class EHeistsInteractionTaskType : uint8
{
	Instant,
	HoldProgress,
	TimingTap,
	Fingerprint,
	CodeMatch,
	Wiring
};

UENUM(BlueprintType)
enum class EHeistsInteractionColor : uint8
{
	Green,
	Yellow,
	Red,
	Blue,
	Gray
};

UENUM(BlueprintType)
enum class EHeistsInteractionProgressBehavior : uint8
{
	ResetOnCancel,
	PreserveOnCancel,
	ResetOnFailure
};

USTRUCT(BlueprintType)
struct HEISTS_API FHeistsInteractionAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	EHeistsInteractionActionId ActionId = EHeistsInteractionActionId::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	EHeistsInteractionTaskType TaskType = EHeistsInteractionTaskType::Instant;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	EHeistsInteractionColor Color = EHeistsInteractionColor::Gray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction", meta = (ClampMin = "0.0"))
	float Duration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	EHeistsInteractionProgressBehavior ProgressBehavior = EHeistsInteractionProgressBehavior::ResetOnCancel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	FGameplayTag RequiredSharedItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	bool bRequiresSharedItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	bool bIsEnabled = true;
};
