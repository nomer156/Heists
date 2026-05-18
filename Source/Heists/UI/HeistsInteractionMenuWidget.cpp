// Copyright 2026 Heists. All Rights Reserved.

#include "UI/HeistsInteractionMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Player/HeistsPlayerController.h"

void UHeistsInteractionMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ActionList && WidgetTree)
	{
		ActionList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ActionList"));
		WidgetTree->RootWidget = ActionList;
	}

	RebuildNativeButtons();
	SetVisibility(ESlateVisibility::Collapsed);
}

void UHeistsInteractionMenuWidget::InitializeMenu(
	AHeistsPlayerController* InController,
	AActor* InTarget,
	const TArray<FHeistsInteractionAction>& InActions)
{
	OwningHeistsController = InController;
	CurrentTarget = InTarget;
	Actions = InActions;
	RebuildNativeButtons();
	SetVisibility(ESlateVisibility::Visible);
	BP_OnMenuUpdated();
}

void UHeistsInteractionMenuWidget::HideMenu()
{
	CurrentTarget.Reset();
	Actions.Reset();
	RebuildNativeButtons();
	SetVisibility(ESlateVisibility::Collapsed);
	BP_OnMenuHidden();
}

void UHeistsInteractionMenuWidget::ConfirmActionByIndex(int32 ActionIndex)
{
	if (!Actions.IsValidIndex(ActionIndex) || !OwningHeistsController)
	{
		return;
	}

	OwningHeistsController->ConfirmInteractionAction(Actions[ActionIndex].ActionId);
}

void UHeistsInteractionMenuWidget::RebuildNativeButtons()
{
	if (!ActionList)
	{
		return;
	}

	ActionList->ClearChildren();

	for (int32 Index = 0; Index < Actions.Num() && Index < 6; ++Index)
	{
		const FHeistsInteractionAction& Action = Actions[Index];

		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Label->SetText(FText::Format(FText::FromString(TEXT("{0}. {1}")), FText::AsNumber(Index + 1), Action.DisplayName));
		Label->SetColorAndOpacity(FSlateColor(GetActionColor(Action.Color)));
		Label->SetMinDesiredWidth(220.f);
		Label->SetJustification(ETextJustify::Left);

		Button->AddChild(Label);
		Button->SetIsEnabled(Action.bIsEnabled);

		switch (Index)
		{
		case 0:
			Button->OnClicked.AddDynamic(this, &UHeistsInteractionMenuWidget::ConfirmSlot1);
			break;
		case 1:
			Button->OnClicked.AddDynamic(this, &UHeistsInteractionMenuWidget::ConfirmSlot2);
			break;
		case 2:
			Button->OnClicked.AddDynamic(this, &UHeistsInteractionMenuWidget::ConfirmSlot3);
			break;
		case 3:
			Button->OnClicked.AddDynamic(this, &UHeistsInteractionMenuWidget::ConfirmSlot4);
			break;
		case 4:
			Button->OnClicked.AddDynamic(this, &UHeistsInteractionMenuWidget::ConfirmSlot5);
			break;
		case 5:
			Button->OnClicked.AddDynamic(this, &UHeistsInteractionMenuWidget::ConfirmSlot6);
			break;
		default:
			break;
		}

		if (UVerticalBoxSlot* ButtonSlot = ActionList->AddChildToVerticalBox(Button))
		{
			ButtonSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
		}
	}
}

FLinearColor UHeistsInteractionMenuWidget::GetActionColor(EHeistsInteractionColor Color) const
{
	switch (Color)
	{
	case EHeistsInteractionColor::Green:
		return FLinearColor(0.1f, 0.9f, 0.25f, 1.f);
	case EHeistsInteractionColor::Yellow:
		return FLinearColor(1.f, 0.82f, 0.1f, 1.f);
	case EHeistsInteractionColor::Red:
		return FLinearColor(1.f, 0.2f, 0.12f, 1.f);
	case EHeistsInteractionColor::Blue:
		return FLinearColor(0.2f, 0.75f, 1.f, 1.f);
	case EHeistsInteractionColor::Gray:
	default:
		return FLinearColor(0.72f, 0.72f, 0.72f, 1.f);
	}
}

void UHeistsInteractionMenuWidget::ConfirmSlot1()
{
	ConfirmActionByIndex(0);
}

void UHeistsInteractionMenuWidget::ConfirmSlot2()
{
	ConfirmActionByIndex(1);
}

void UHeistsInteractionMenuWidget::ConfirmSlot3()
{
	ConfirmActionByIndex(2);
}

void UHeistsInteractionMenuWidget::ConfirmSlot4()
{
	ConfirmActionByIndex(3);
}

void UHeistsInteractionMenuWidget::ConfirmSlot5()
{
	ConfirmActionByIndex(4);
}

void UHeistsInteractionMenuWidget::ConfirmSlot6()
{
	ConfirmActionByIndex(5);
}
