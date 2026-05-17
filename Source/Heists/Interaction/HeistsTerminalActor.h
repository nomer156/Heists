// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/HeistsInteractableActorBase.h"
#include "HeistsTerminalActor.generated.h"

UCLASS(Blueprintable)
class HEISTS_API AHeistsTerminalActor : public AHeistsInteractableActorBase
{
	GENERATED_BODY()

public:
	AHeistsTerminalActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void ApplyInteractionResult(AActor* Interactor, EHeistsInteractionActionId ActionId) override;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|Terminal")
	bool bWasCompleted = false;
};
