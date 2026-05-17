// Copyright 2026 Heists. All Rights Reserved.

#include "UI/HeistsHUD.h"

AHeistsHUD::AHeistsHUD()
{
}

void AHeistsHUD::BeginPlay()
{
	Super::BeginPlay();
	// Показываем игровой HUD при старте (только на локальном клиенте)
	ShowGameHUD();
}

void AHeistsHUD::DrawHUD()
{
	Super::DrawHUD();
	// Debug-рисование можно добавить здесь (временно для разработки)
}

void AHeistsHUD::ShowGameHUD()
{
	BP_ShowGameHUD();
}

void AHeistsHUD::ShowPauseMenu()
{
	BP_ShowPauseMenu();
}

void AHeistsHUD::ShowMissionResults(bool bSuccess)
{
	BP_ShowMissionResults(bSuccess);
}
