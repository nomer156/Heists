// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/HeistsInteractable.h"
#include "HeistsInteractableActorBase.generated.h"

UCLASS(Abstract, Blueprintable)
class HEISTS_API AHeistsInteractableActorBase : public AActor, public IHeistsInteractable
{
	GENERATED_BODY()

public:
	AHeistsInteractableActorBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual TArray<FHeistsInteractionAction> GetAvailableInteractionActions_Implementation(AActor* Interactor) const override;
	virtual bool CanInteract_Implementation(AActor* Interactor, EHeistsInteractionActionId ActionId) const override;
	virtual void BeginInteract_Implementation(AActor* Interactor, EHeistsInteractionActionId ActionId) override;
	virtual void CancelInteract_Implementation(AActor* Interactor) override;
	virtual void CommitInteract_Implementation(AActor* Interactor, EHeistsInteractionActionId ActionId) override;

	UFUNCTION(BlueprintPure, Category = "Heists|Interaction")
	bool IsBusy() const { return ActiveInteractor != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Heists|Interaction")
	float GetInteractionProgress() const { return InteractionProgress; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	TArray<FHeistsInteractionAction> Actions;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ActiveInteractor, Category = "Heists|Interaction")
	TObjectPtr<AActor> ActiveInteractor;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Progress, Category = "Heists|Interaction")
	float InteractionProgress = 0.f;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|Interaction")
	EHeistsInteractionActionId ActiveActionId = EHeistsInteractionActionId::None;

	FTimerHandle InteractionProgressTimerHandle;
	float ActiveInteractionDuration = 0.f;

	const FHeistsInteractionAction* FindAction(EHeistsInteractionActionId ActionId) const;
	virtual void ApplyInteractionResult(AActor* Interactor, EHeistsInteractionActionId ActionId);
	void AdvanceInteractionProgress();
	void ClearInteractionState();

	UFUNCTION()
	void OnRep_ActiveInteractor();

	UFUNCTION()
	void OnRep_Progress();

	UFUNCTION(BlueprintImplementableEvent, Category = "Heists|Interaction")
	void BP_OnBusyStateChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "Heists|Interaction")
	void BP_OnProgressChanged();
};
