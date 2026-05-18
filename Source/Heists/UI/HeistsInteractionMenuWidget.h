// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interaction/HeistsInteractionTypes.h"
#include "HeistsInteractionMenuWidget.generated.h"

class AHeistsPlayerController;
class UVerticalBox;

UCLASS(Blueprintable)
class HEISTS_API UHeistsInteractionMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Heists|Interaction Menu")
	void InitializeMenu(AHeistsPlayerController* InController, AActor* InTarget, const TArray<FHeistsInteractionAction>& InActions);

	UFUNCTION(BlueprintCallable, Category = "Heists|Interaction Menu")
	void HideMenu();

	UFUNCTION(BlueprintCallable, Category = "Heists|Interaction Menu")
	void ConfirmActionByIndex(int32 ActionIndex);

	UFUNCTION(BlueprintPure, Category = "Heists|Interaction Menu")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

	UFUNCTION(BlueprintPure, Category = "Heists|Interaction Menu")
	const TArray<FHeistsInteractionAction>& GetActions() const { return Actions; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Heists|Interaction Menu")
	void BP_OnMenuUpdated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Heists|Interaction Menu")
	void BP_OnMenuHidden();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Heists|Interaction Menu")
	TObjectPtr<AHeistsPlayerController> OwningHeistsController;

	UPROPERTY(BlueprintReadOnly, Category = "Heists|Interaction Menu")
	TWeakObjectPtr<AActor> CurrentTarget;

	UPROPERTY(BlueprintReadOnly, Category = "Heists|Interaction Menu")
	TArray<FHeistsInteractionAction> Actions;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Heists|Interaction Menu")
	TObjectPtr<UVerticalBox> ActionList;

	void RebuildNativeButtons();
	FLinearColor GetActionColor(EHeistsInteractionColor Color) const;

	UFUNCTION()
	void ConfirmSlot1();

	UFUNCTION()
	void ConfirmSlot2();

	UFUNCTION()
	void ConfirmSlot3();

	UFUNCTION()
	void ConfirmSlot4();

	UFUNCTION()
	void ConfirmSlot5();

	UFUNCTION()
	void ConfirmSlot6();
};
