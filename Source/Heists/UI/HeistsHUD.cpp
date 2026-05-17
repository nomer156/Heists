// Copyright 2026 Heists. All Rights Reserved.

#include "UI/HeistsHUD.h"

#include "Character/HeistsRobber.h"
#include "Game/HeistsGameState.h"
#include "Interaction/HeistsInteractableActorBase.h"
#include "Loot/HeistsLootBag.h"
#include "Player/HeistsPlayerController.h"
#include "Engine/Canvas.h"

namespace
{
FColor GetDebugActionColor(EHeistsInteractionColor Color)
{
	switch (Color)
	{
	case EHeistsInteractionColor::Green:
		return FColor::Green;
	case EHeistsInteractionColor::Yellow:
		return FColor::Yellow;
	case EHeistsInteractionColor::Red:
		return FColor::Red;
	case EHeistsInteractionColor::Blue:
		return FColor::Cyan;
	case EHeistsInteractionColor::Gray:
	default:
		return FColor::Silver;
	}
}
}

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

	AHeistsPlayerController* HeistsPC = Cast<AHeistsPlayerController>(PlayerOwner);
	if (!HeistsPC)
	{
		return;
	}

	float X = Canvas ? Canvas->ClipX - 420.f : 900.f;
	float Y = 80.f;
	constexpr float LineHeight = 20.f;

	DrawText(TEXT("HEISTS DEBUG HUD"), FColor::White, X, Y);
	Y += LineHeight * 1.5f;

	AActor* RadialTarget = HeistsPC->GetRadialTarget();
	if (HeistsPC->IsInteractionRadialOpen() && RadialTarget)
	{
		DrawText(FString::Printf(TEXT("Target: %s"), *GetNameSafe(RadialTarget)), FColor::White, X, Y);
		Y += LineHeight;

		const TArray<FHeistsInteractionAction> Actions = HeistsPC->GetRadialActions();
		for (int32 Index = 0; Index < Actions.Num(); ++Index)
		{
			const FHeistsInteractionAction& Action = Actions[Index];
			DrawText(
				FString::Printf(TEXT("%d: %s"), Index + 1, *Action.DisplayName.ToString()),
				GetDebugActionColor(Action.Color),
				X,
				Y);
			Y += LineHeight;
		}
	}
	else
	{
		DrawText(TEXT("Action: press/tap interact near target"), FColor::Silver, X, Y);
		Y += LineHeight;
	}

	if (AHeistsInteractableActorBase* InteractableTarget = Cast<AHeistsInteractableActorBase>(RadialTarget))
	{
		const float Progress = InteractableTarget->GetInteractionProgress();
		if (Progress > 0.f)
		{
			DrawText(FString::Printf(TEXT("Progress: %.0f%%"), Progress * 100.f), FColor::Yellow, X, Y);
			Y += LineHeight;
		}
	}

	if (AHeistsRobber* Robber = Cast<AHeistsRobber>(HeistsPC->GetPawn()))
	{
		const AHeistsLootBag* CarriedBag = Robber->GetCarriedLootBag();
		DrawText(
			CarriedBag
				? FString::Printf(TEXT("Carrying bag: value=%d weight=%.1f"), CarriedBag->GetLootValue(), CarriedBag->GetWeight())
				: TEXT("Carrying bag: none"),
			CarriedBag ? FColor::Yellow : FColor::Silver,
			X,
			Y);
		Y += LineHeight;
	}

	if (const AHeistsGameState* HeistsGameState = GetWorld() ? GetWorld()->GetGameState<AHeistsGameState>() : nullptr)
	{
		DrawText(FString::Printf(TEXT("Loot collected: %d"), HeistsGameState->GetLootCollected()), FColor::Green, X, Y);
		Y += LineHeight;

		const FString SharedItems = HeistsGameState->GetSharedCrewItems().ToStringSimple();
		DrawText(
			SharedItems.IsEmpty() ? TEXT("Shared items: none") : FString::Printf(TEXT("Shared items: %s"), *SharedItems),
			FColor::Cyan,
			X,
			Y);
	}
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
