// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HeistsRoleTypes.generated.h"

UENUM(BlueprintType)
enum class EHeistsCrewRole : uint8
{
	None,
	Coordinator,
	Hacker,
	Breaker,
	Scout,
	Driver
};

USTRUCT(BlueprintType)
struct HEISTS_API FHeistsRoleTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Roles")
	EHeistsCrewRole Role = EHeistsCrewRole::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Roles", meta = (ClampMin = "0.1"))
	float HackDurationMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Roles", meta = (ClampMin = "0.1"))
	float ForceDurationMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Roles", meta = (ClampMin = "0.1"))
	float QuietInteractionMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Roles", meta = (ClampMin = "0.0"))
	float ContextScanRangeBonus = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Roles", meta = (ClampMin = "0.0"))
	float TeamInfoRangeBonus = 0.f;
};
