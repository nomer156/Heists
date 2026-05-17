// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/HeistsInteractionTypes.h"
#include "HeistsInteractionComponent.generated.h"

class IHeistsInteractable;

UCLASS(ClassGroup = (Heists), meta = (BlueprintSpawnableComponent))
class HEISTS_API UHeistsInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHeistsInteractionComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Heists|Interaction")
	AActor* FindBestInteractable() const;

	UFUNCTION(BlueprintCallable, Category = "Heists|Interaction")
	TArray<FHeistsInteractionAction> GetAvailableActionsForTarget(AActor* Target) const;

	UFUNCTION(BlueprintCallable, Category = "Heists|Interaction")
	void RequestPrimaryInteraction();

	UFUNCTION(BlueprintCallable, Category = "Heists|Interaction")
	void RequestInteraction(AActor* Target, EHeistsInteractionActionId ActionId);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestInteraction(AActor* Target, EHeistsInteractionActionId ActionId);

	UFUNCTION(Server, Reliable)
	void Server_CancelInteraction();

	UFUNCTION(BlueprintPure, Category = "Heists|Interaction")
	AActor* GetActiveInteractionTarget() const { return ActiveInteractionTarget; }

	UFUNCTION(BlueprintPure, Category = "Heists|Interaction")
	EHeistsInteractionActionId GetActiveActionId() const { return ActiveActionId; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Interaction", meta = (ClampMin = "0.0"))
	float InteractionRadius = 250.f;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|Interaction")
	TObjectPtr<AActor> ActiveInteractionTarget;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|Interaction")
	EHeistsInteractionActionId ActiveActionId = EHeistsInteractionActionId::None;

	bool IsTargetInRange(AActor* Target) const;
	static bool IsActionAvailable(const TArray<FHeistsInteractionAction>& Actions, EHeistsInteractionActionId ActionId);
};
