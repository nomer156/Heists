// Copyright 2026 Heists. All Rights Reserved.

#include "UI/HeistsHUD.h"

#include "Character/HeistsCoverComponent.h"
#include "Character/HeistsRobber.h"
#include "Game/HeistsGameState.h"
#include "Interaction/HeistsInteractableActorBase.h"
#include "Loot/HeistsLootBag.h"
#include "Player/HeistsPlayerController.h"
#include "UI/HeistsInteractionMenuWidget.h"
#include "UI/HeistsMobileHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Canvas.h"
#include "UObject/ConstructorHelpers.h"

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
	InteractionMenuWidgetClass = UHeistsInteractionMenuWidget::StaticClass();
	MobileHUDWidgetClass = UHeistsMobileHUDWidget::StaticClass();

	static ConstructorHelpers::FClassFinder<UHeistsInteractionMenuWidget> InteractionMenuWidgetFinder(
		TEXT("/Game/UI/WBP_InteractionMenu"));
	if (InteractionMenuWidgetFinder.Succeeded())
	{
		InteractionMenuWidgetClass = InteractionMenuWidgetFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UHeistsMobileHUDWidget> MobileHUDWidgetFinder(
		TEXT("/Game/UI/WBP_MobileHUD"));
	if (MobileHUDWidgetFinder.Succeeded())
	{
		MobileHUDWidgetClass = MobileHUDWidgetFinder.Class;
	}
}

void AHeistsHUD::BeginPlay()
{
	Super::BeginPlay();
	// Показываем игровой HUD при старте (только на локальном клиенте)
	ShowGameHUD();
	RefreshMobileHUD();
	RefreshInteractionMenu();
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
		const UEnum* CrewRoleEnum = StaticEnum<EHeistsCrewRole>();
		const FString CrewRoleName = CrewRoleEnum
			? CrewRoleEnum->GetNameStringByValue(static_cast<int64>(Robber->GetCrewRole()))
			: TEXT("Unknown");
		const UHeistsCoverComponent* CoverComponent = Robber->GetCoverComponent();
		DrawText(
			FString::Printf(
				TEXT("Role: %s | Cover: %s"),
				*CrewRoleName,
				CoverComponent ? *CoverComponent->GetCoverStateName().ToString() : TEXT("None")),
			FColor::White,
			X,
			Y);
		Y += LineHeight;

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
		Y += LineHeight;

		const FHeistsPrototypeMissionResult MissionResult = HeistsGameState->GetPrototypeMissionResult();
		const TCHAR* MissionState = MissionResult.bMissionCompleted
			? TEXT("Completed")
			: MissionResult.bMissionFailed
				? TEXT("Failed")
				: MissionResult.bMissionActive
					? TEXT("Active")
					: TEXT("Idle");
		DrawText(
			FString::Printf(
				TEXT("Mission: %s | Loot: %d | Shared Items: %d"),
				MissionState,
				MissionResult.DeliveredLootValue,
				MissionResult.SharedItemsAcquired),
			FColor::White,
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

void AHeistsHUD::RefreshInteractionMenu()
{
	AHeistsPlayerController* HeistsPC = Cast<AHeistsPlayerController>(PlayerOwner);
	if (!HeistsPC)
	{
		return;
	}

	if (!InteractionMenuWidget && InteractionMenuWidgetClass)
	{
		InteractionMenuWidget = CreateWidget<UHeistsInteractionMenuWidget>(HeistsPC, InteractionMenuWidgetClass);
		if (InteractionMenuWidget)
		{
			InteractionMenuWidget->AddToViewport(20);
		}
	}

	if (!InteractionMenuWidget)
	{
		return;
	}

	if (HeistsPC->IsInteractionRadialOpen() && HeistsPC->GetRadialTarget())
	{
		InteractionMenuWidget->InitializeMenu(HeistsPC, HeistsPC->GetRadialTarget(), HeistsPC->GetRadialActions());
	}
	else
	{
		InteractionMenuWidget->HideMenu();
	}
}

void AHeistsHUD::RefreshMobileHUD()
{
	AHeistsPlayerController* HeistsPC = Cast<AHeistsPlayerController>(PlayerOwner);
	if (!HeistsPC)
	{
		return;
	}

	if (!MobileHUDWidget && MobileHUDWidgetClass)
	{
		MobileHUDWidget = CreateWidget<UHeistsMobileHUDWidget>(HeistsPC, MobileHUDWidgetClass);
		if (MobileHUDWidget)
		{
			MobileHUDWidget->AddToViewport(10);
			MobileHUDWidget->InitializeMobileHUD(HeistsPC);
		}
	}

	if (MobileHUDWidget)
	{
		MobileHUDWidget->ApplyLayoutMode(HeistsPC->GetCurrentMobileLayoutMode());
	}
}
