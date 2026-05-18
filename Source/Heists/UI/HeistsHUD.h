// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HeistsHUD.generated.h"

/**
 * AHeistsHUD
 *
 * Базовый HUD. Логика отображения — в Common UI виджетах (BP).
 * C++ отвечает за: управление стеком виджетов, показ/скрытие UI элементов.
 */
UCLASS()
class HEISTS_API AHeistsHUD : public AHUD
{
	GENERATED_BODY()

public:
	AHeistsHUD();

	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

	// Показать главный игровой HUD
	UFUNCTION(BlueprintCallable, Category = "Heists|HUD")
	void ShowGameHUD();

	// Показать экран паузы
	UFUNCTION(BlueprintCallable, Category = "Heists|HUD")
	void ShowPauseMenu();

	// Показать результаты миссии
	UFUNCTION(BlueprintCallable, Category = "Heists|HUD")
	void ShowMissionResults(bool bSuccess);

	UFUNCTION(BlueprintCallable, Category = "Heists|HUD|Interaction")
	void RefreshInteractionMenu();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|HUD|Interaction")
	TSubclassOf<class UHeistsInteractionMenuWidget> InteractionMenuWidgetClass;

	// Blueprint-события для создания виджетов
	UFUNCTION(BlueprintImplementableEvent, Category = "Heists|HUD")
	void BP_ShowGameHUD();

	UFUNCTION(BlueprintImplementableEvent, Category = "Heists|HUD")
	void BP_ShowPauseMenu();

	UFUNCTION(BlueprintImplementableEvent, Category = "Heists|HUD")
	void BP_ShowMissionResults(bool bSuccess);

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Heists|HUD|Interaction")
	TObjectPtr<class UHeistsInteractionMenuWidget> InteractionMenuWidget;
};
