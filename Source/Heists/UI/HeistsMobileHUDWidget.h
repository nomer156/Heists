// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/HeistsMobileLayoutTypes.h"
#include "HeistsMobileHUDWidget.generated.h"

class AHeistsPlayerController;
class UButton;
class UPanelWidget;

UCLASS(Blueprintable)
class HEISTS_API UHeistsMobileHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Heists|Mobile HUD")
	void InitializeMobileHUD(AHeistsPlayerController* InController);

	UFUNCTION(BlueprintCallable, Category = "Heists|Mobile HUD")
	void ApplyLayoutMode(EHeistsMobileLayoutMode NewLayoutMode);

	UFUNCTION(BlueprintPure, Category = "Heists|Mobile HUD")
	EHeistsMobileLayoutMode GetLayoutMode() const { return LayoutMode; }

	UFUNCTION(BlueprintPure, Category = "Heists|Mobile HUD")
	static TArray<FName> GetRequiredDesignerWidgetNames();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Heists|Mobile HUD")
	TObjectPtr<AHeistsPlayerController> OwningHeistsController;

	UPROPERTY(BlueprintReadOnly, Category = "Heists|Mobile HUD")
	EHeistsMobileLayoutMode LayoutMode = EHeistsMobileLayoutMode::Portrait;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Heists|Mobile HUD")
	TObjectPtr<UPanelWidget> Panel_Objectives;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Heists|Mobile HUD")
	TObjectPtr<UPanelWidget> Panel_QuickCommands;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Heists|Mobile HUD")
	TObjectPtr<UPanelWidget> Panel_PortraitRoot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Heists|Mobile HUD")
	TObjectPtr<UPanelWidget> Panel_LandscapeRoot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Heists|Mobile HUD")
	TObjectPtr<UButton> Button_Interact;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Heists|Mobile HUD")
	TObjectPtr<UButton> Button_DropBag;

	UFUNCTION()
	void HandleInteractClicked();

	UFUNCTION()
	void HandleDropBagClicked();

	void BuildNativeFallbackLayout();
};
