// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/HeistsInteractableActorBase.h"
#include "HeistsDoorActor.generated.h"

UCLASS(Blueprintable)
class HEISTS_API AHeistsDoorActor : public AHeistsInteractableActorBase
{
	GENERATED_BODY()

public:
	AHeistsDoorActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Heists|Door")
	bool IsOpen() const { return bIsOpen; }

	UFUNCTION(BlueprintPure, Category = "Heists|Door")
	bool IsLocked() const { return bIsLocked; }

protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DoorState, Category = "Heists|Door")
	bool bIsOpen = false;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DoorState, Category = "Heists|Door")
	bool bIsLocked = false;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DoorState, Category = "Heists|Door")
	bool bIsPeeked = false;

	virtual void ApplyInteractionResult(AActor* Interactor, EHeistsInteractionActionId ActionId) override;

	UFUNCTION()
	void OnRep_DoorState();

	UFUNCTION(BlueprintImplementableEvent, Category = "Heists|Door")
	void BP_OnDoorStateChanged();
};
