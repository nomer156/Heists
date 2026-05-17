// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/HeistsInteractableActorBase.h"
#include "HeistsLootBag.generated.h"

class AHeistsRobber;

UCLASS(Blueprintable)
class HEISTS_API AHeistsLootBag : public AHeistsInteractableActorBase
{
	GENERATED_BODY()

public:
	AHeistsLootBag();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Heists|Loot")
	int32 GetLootValue() const { return LootValue; }

	UFUNCTION(BlueprintPure, Category = "Heists|Loot")
	float GetWeight() const { return Weight; }

	UFUNCTION(BlueprintCallable, Category = "Heists|Loot")
	bool TryPickup(AHeistsRobber* Robber);

	UFUNCTION(BlueprintCallable, Category = "Heists|Loot")
	void DropFromCarrier(const FVector& DropLocation);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Heists|Loot", meta = (ClampMin = "0"))
	int32 LootValue = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Heists|Loot", meta = (ClampMin = "0.0"))
	float Weight = 20.f;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CarryState, Category = "Heists|Loot")
	bool bIsCarried = false;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CarryState, Category = "Heists|Loot")
	TObjectPtr<AHeistsRobber> Carrier;

	virtual void ApplyInteractionResult(AActor* Interactor, EHeistsInteractionActionId ActionId) override;

	UFUNCTION()
	void OnRep_CarryState();

	void ApplyCarryVisualState();
};
