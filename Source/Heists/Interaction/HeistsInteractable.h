// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interaction/HeistsInteractionTypes.h"
#include "HeistsInteractable.generated.h"

UINTERFACE(BlueprintType)
class HEISTS_API UHeistsInteractable : public UInterface
{
	GENERATED_BODY()
};

class HEISTS_API IHeistsInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Heists|Interaction")
	TArray<FHeistsInteractionAction> GetAvailableInteractionActions(AActor* Interactor) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Heists|Interaction")
	bool CanInteract(AActor* Interactor, EHeistsInteractionActionId ActionId) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Heists|Interaction")
	void BeginInteract(AActor* Interactor, EHeistsInteractionActionId ActionId);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Heists|Interaction")
	void CancelInteract(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Heists|Interaction")
	void CommitInteract(AActor* Interactor, EHeistsInteractionActionId ActionId);
};
