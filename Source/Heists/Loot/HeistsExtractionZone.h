// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/HeistsInteractableActorBase.h"
#include "HeistsExtractionZone.generated.h"

UCLASS(Blueprintable)
class HEISTS_API AHeistsExtractionZone : public AHeistsInteractableActorBase
{
	GENERATED_BODY()

public:
	AHeistsExtractionZone();

protected:
	virtual void ApplyInteractionResult(AActor* Interactor, EHeistsInteractionActionId ActionId) override;
};
