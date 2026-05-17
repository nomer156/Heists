// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/HeistsInteractableActorBase.h"
#include "HeistsLootContainer.generated.h"

class AHeistsLootBag;

UCLASS(Blueprintable)
class HEISTS_API AHeistsLootContainer : public AHeistsInteractableActorBase
{
	GENERATED_BODY()

public:
	AHeistsLootContainer();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Loot")
	TSubclassOf<AHeistsLootBag> LootBagClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Loot")
	FVector LootSpawnOffset = FVector(100.f, 0.f, 0.f);

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|Loot")
	bool bWasSearched = false;

	virtual void ApplyInteractionResult(AActor* Interactor, EHeistsInteractionActionId ActionId) override;
};
