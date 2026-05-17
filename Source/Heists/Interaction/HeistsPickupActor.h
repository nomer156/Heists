// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Interaction/HeistsInteractableActorBase.h"
#include "HeistsPickupActor.generated.h"

UCLASS(Blueprintable)
class HEISTS_API AHeistsPickupActor : public AHeistsInteractableActorBase
{
	GENERATED_BODY()

public:
	AHeistsPickupActor();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|SharedItems")
	FGameplayTag SharedItemTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|SharedItems")
	bool bDestroyOnPickup = true;

	virtual void ApplyInteractionResult(AActor* Interactor, EHeistsInteractionActionId ActionId) override;
};
