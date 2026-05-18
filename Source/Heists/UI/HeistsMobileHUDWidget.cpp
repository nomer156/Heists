// Copyright 2026 Heists. All Rights Reserved.

#include "UI/HeistsMobileHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Player/HeistsPlayerController.h"

void UHeistsMobileHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildNativeFallbackLayout();
	}

	if (Button_Interact)
	{
		Button_Interact->OnClicked.RemoveAll(this);
		Button_Interact->OnClicked.AddDynamic(this, &UHeistsMobileHUDWidget::HandleInteractClicked);
	}

	if (Button_DropBag)
	{
		Button_DropBag->OnClicked.RemoveAll(this);
		Button_DropBag->OnClicked.AddDynamic(this, &UHeistsMobileHUDWidget::HandleDropBagClicked);
	}
}

void UHeistsMobileHUDWidget::InitializeMobileHUD(AHeistsPlayerController* InController)
{
	OwningHeistsController = InController;
	if (OwningHeistsController)
	{
		ApplyLayoutMode(OwningHeistsController->GetCurrentMobileLayoutMode());
	}
}

void UHeistsMobileHUDWidget::ApplyLayoutMode(EHeistsMobileLayoutMode NewLayoutMode)
{
	LayoutMode = NewLayoutMode;

	if (Panel_PortraitRoot)
	{
		Panel_PortraitRoot->SetVisibility(
			LayoutMode == EHeistsMobileLayoutMode::Portrait ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Panel_LandscapeRoot)
	{
		Panel_LandscapeRoot->SetVisibility(
			LayoutMode == EHeistsMobileLayoutMode::Landscape ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

TArray<FName> UHeistsMobileHUDWidget::GetRequiredDesignerWidgetNames()
{
	return {
		TEXT("Panel_Objectives"),
		TEXT("Panel_QuickCommands"),
		TEXT("Panel_PortraitRoot"),
		TEXT("Panel_LandscapeRoot"),
		TEXT("Button_Interact"),
		TEXT("Button_DropBag"),
		TEXT("ActionList")
	};
}

void UHeistsMobileHUDWidget::HandleInteractClicked()
{
	if (OwningHeistsController)
	{
		OwningHeistsController->OpenInteractionRadial();
	}
}

void UHeistsMobileHUDWidget::HandleDropBagClicked()
{
	if (OwningHeistsController)
	{
		OwningHeistsController->DropCarriedLoot();
	}
}

void UHeistsMobileHUDWidget::BuildNativeFallbackLayout()
{
	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	UCanvasPanel* PortraitRoot = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Panel_PortraitRoot"));
	Panel_PortraitRoot = PortraitRoot;
	if (UCanvasPanelSlot* PortraitSlot = RootCanvas->AddChildToCanvas(PortraitRoot))
	{
		PortraitSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		PortraitSlot->SetOffsets(FMargin(0.f));
	}

	UCanvasPanel* LandscapeRoot = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Panel_LandscapeRoot"));
	Panel_LandscapeRoot = LandscapeRoot;
	if (UCanvasPanelSlot* LandscapeSlot = RootCanvas->AddChildToCanvas(LandscapeRoot))
	{
		LandscapeSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		LandscapeSlot->SetOffsets(FMargin(0.f));
	}

	UVerticalBox* ObjectivesBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Panel_Objectives"));
	Panel_Objectives = ObjectivesBox;
	if (UCanvasPanelSlot* ObjectivesSlot = PortraitRoot->AddChildToCanvas(ObjectivesBox))
	{
		ObjectivesSlot->SetAnchors(FAnchors(0.05f, 0.02f, 0.95f, 0.02f));
		ObjectivesSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 92.f));
	}

	UTextBlock* ObjectiveText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Objectives_Debug"));
	ObjectiveText->SetText(FText::FromString(TEXT("Mission debug")));
	ObjectiveText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	ObjectivesBox->AddChildToVerticalBox(ObjectiveText);

	UHorizontalBox* QuickCommandsBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Panel_QuickCommands"));
	Panel_QuickCommands = QuickCommandsBox;
	if (UCanvasPanelSlot* CommandsSlot = PortraitRoot->AddChildToCanvas(QuickCommandsBox))
	{
		CommandsSlot->SetAnchors(FAnchors(0.16f, 0.80f, 0.84f, 0.80f));
		CommandsSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 96.f));
	}

	Button_Interact = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Button_Interact"));
	UTextBlock* InteractLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Interact"));
	InteractLabel->SetText(FText::FromString(TEXT("Interact")));
	Button_Interact->AddChild(InteractLabel);
	if (UHorizontalBoxSlot* InteractSlot = QuickCommandsBox->AddChildToHorizontalBox(Button_Interact))
	{
		InteractSlot->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));
	}

	Button_DropBag = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Button_DropBag"));
	UTextBlock* DropLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_DropBag"));
	DropLabel->SetText(FText::FromString(TEXT("Drop")));
	Button_DropBag->AddChild(DropLabel);
	QuickCommandsBox->AddChildToHorizontalBox(Button_DropBag);

	UTextBlock* LandscapeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Landscape_Debug"));
	LandscapeText->SetText(FText::FromString(TEXT("Landscape debug HUD")));
	LandscapeText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	if (UCanvasPanelSlot* LandscapeTextSlot = LandscapeRoot->AddChildToCanvas(LandscapeText))
	{
		LandscapeTextSlot->SetAnchors(FAnchors(0.05f, 0.05f, 0.05f, 0.05f));
		LandscapeTextSlot->SetOffsets(FMargin(0.f, 0.f, 240.f, 48.f));
	}
}
