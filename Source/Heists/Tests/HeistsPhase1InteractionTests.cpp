// Copyright 2026 Heists. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Game/HeistsGameState.h"
#include "Character/HeistsRobber.h"
#include "Player/HeistsPlayerController.h"
#include "Interaction/HeistsInteractable.h"
#include "Interaction/HeistsInteractionComponent.h"
#include "Interaction/HeistsDoorActor.h"
#include "Interaction/HeistsInteractionTypes.h"
#include "Interaction/HeistsPickupActor.h"
#include "Interaction/HeistsTerminalActor.h"
#include "Loot/HeistsExtractionZone.h"
#include "Loot/HeistsLootBag.h"
#include "Loot/HeistsLootContainer.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHeistsInteractionActionDefaultsTest,
	"Heists.Phase1.Interaction.ActionDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeistsInteractionActionDefaultsTest::RunTest(const FString& Parameters)
{
	FHeistsInteractionAction OpenAction;
	OpenAction.ActionId = EHeistsInteractionActionId::Open;
	OpenAction.DisplayName = FText::FromString(TEXT("Open"));
	OpenAction.Color = EHeistsInteractionColor::Green;
	OpenAction.TaskType = EHeistsInteractionTaskType::Instant;

	TestEqual(TEXT("Open action id is Open"), OpenAction.ActionId, EHeistsInteractionActionId::Open);
	TestEqual(TEXT("Open action is green"), OpenAction.Color, EHeistsInteractionColor::Green);
	TestEqual(TEXT("Instant action has no duration"), OpenAction.Duration, 0.f);

	FHeistsInteractionAction HackAction;
	HackAction.ActionId = EHeistsInteractionActionId::Hack;
	HackAction.Color = EHeistsInteractionColor::Yellow;
	HackAction.TaskType = EHeistsInteractionTaskType::HoldProgress;
	HackAction.Duration = 3.f;
	HackAction.ProgressBehavior = EHeistsInteractionProgressBehavior::PreserveOnCancel;

	TestEqual(TEXT("Hack action is yellow"), HackAction.Color, EHeistsInteractionColor::Yellow);
	TestEqual(TEXT("Hack action preserves progress"), HackAction.ProgressBehavior, EHeistsInteractionProgressBehavior::PreserveOnCancel);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHeistsSharedCrewItemsTest,
	"Heists.Phase1.SharedItems.GameStateContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeistsSharedCrewItemsTest::RunTest(const FString& Parameters)
{
	const UClass* GameStateClass = AHeistsGameState::StaticClass();
	TestNotNull(TEXT("AHeistsGameState class exists"), GameStateClass);

	TestNotNull(
		TEXT("GameState exposes replicated SharedCrewItems"),
		FindFProperty<FStructProperty>(GameStateClass, TEXT("SharedCrewItems")));

	TestNotNull(
		TEXT("GameState exposes HasSharedCrewItem"),
		GameStateClass->FindFunctionByName(TEXT("HasSharedCrewItem")));

	TestNotNull(
		TEXT("GameState exposes AddSharedCrewItem"),
		GameStateClass->FindFunctionByName(TEXT("AddSharedCrewItem")));

	TestNotNull(
		TEXT("GameState exposes GetSharedCrewItems"),
		GameStateClass->FindFunctionByName(TEXT("GetSharedCrewItems")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHeistsInteractionComponentContractTest,
	"Heists.Phase1.Interaction.ComponentContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeistsInteractionComponentContractTest::RunTest(const FString& Parameters)
{
	const UClass* InteractableClass = UHeistsInteractable::StaticClass();
	TestNotNull(TEXT("IHeistsInteractable interface exists"), InteractableClass);

	const UClass* ComponentClass = UHeistsInteractionComponent::StaticClass();
	TestNotNull(TEXT("UHeistsInteractionComponent class exists"), ComponentClass);
	TestNotNull(TEXT("Component exposes FindBestInteractable"), ComponentClass->FindFunctionByName(TEXT("FindBestInteractable")));
	TestNotNull(TEXT("Component exposes RequestPrimaryInteraction"), ComponentClass->FindFunctionByName(TEXT("RequestPrimaryInteraction")));
	TestNotNull(TEXT("Component exposes Server_RequestInteraction"), ComponentClass->FindFunctionByName(TEXT("Server_RequestInteraction")));
	TestNotNull(TEXT("Component exposes Server_CancelInteraction"), ComponentClass->FindFunctionByName(TEXT("Server_CancelInteraction")));

	const AHeistsRobber* RobberCDO = GetDefault<AHeistsRobber>();
	TestNotNull(TEXT("AHeistsRobber CDO exists"), RobberCDO);
	TestNotNull(TEXT("AHeistsRobber owns interaction component"), RobberCDO ? RobberCDO->FindComponentByClass<UHeistsInteractionComponent>() : nullptr);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHeistsInteractableActorDefaultsTest,
	"Heists.Phase1.Interaction.ActorDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeistsInteractableActorDefaultsTest::RunTest(const FString& Parameters)
{
	AHeistsDoorActor* DoorCDO = GetMutableDefault<AHeistsDoorActor>();
	TestNotNull(TEXT("AHeistsDoorActor CDO exists"), DoorCDO);
	TestTrue(TEXT("Door implements interactable"), DoorCDO && DoorCDO->GetClass()->ImplementsInterface(UHeistsInteractable::StaticClass()));

	const TArray<FHeistsInteractionAction> DoorActions =
		DoorCDO ? IHeistsInteractable::Execute_GetAvailableInteractionActions(DoorCDO, nullptr) : TArray<FHeistsInteractionAction>();
	TestTrue(TEXT("Door exposes multiple actions"), DoorActions.Num() >= 6);
	TestTrue(TEXT("Door exposes Open"), DoorActions.ContainsByPredicate([](const FHeistsInteractionAction& Action) { return Action.ActionId == EHeistsInteractionActionId::Open; }));
	TestTrue(TEXT("Door exposes Breach"), DoorActions.ContainsByPredicate([](const FHeistsInteractionAction& Action) { return Action.ActionId == EHeistsInteractionActionId::Breach && Action.Color == EHeistsInteractionColor::Red; }));

	AHeistsTerminalActor* TerminalCDO = GetMutableDefault<AHeistsTerminalActor>();
	TestNotNull(TEXT("AHeistsTerminalActor CDO exists"), TerminalCDO);
	TestTrue(TEXT("Terminal implements interactable"), TerminalCDO && TerminalCDO->GetClass()->ImplementsInterface(UHeistsInteractable::StaticClass()));

	const TArray<FHeistsInteractionAction> TerminalActions =
		TerminalCDO ? IHeistsInteractable::Execute_GetAvailableInteractionActions(TerminalCDO, nullptr) : TArray<FHeistsInteractionAction>();
	TestEqual(TEXT("Terminal exposes one configured action"), TerminalActions.Num(), 1);
	if (TerminalActions.Num() == 1)
	{
		TestEqual(TEXT("Terminal default action is Hack"), TerminalActions[0].ActionId, EHeistsInteractionActionId::Hack);
		TestEqual(TEXT("Terminal default task is HoldProgress"), TerminalActions[0].TaskType, EHeistsInteractionTaskType::HoldProgress);
		TestEqual(TEXT("Terminal action is yellow"), TerminalActions[0].Color, EHeistsInteractionColor::Yellow);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHeistsLootCarryContractTest,
	"Heists.Phase1.Loot.CarryContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeistsLootCarryContractTest::RunTest(const FString& Parameters)
{
	const UClass* RobberClass = AHeistsRobber::StaticClass();
	TestNotNull(TEXT("Robber exposes CanCarryLootBag"), RobberClass->FindFunctionByName(TEXT("CanCarryLootBag")));
	TestNotNull(TEXT("Robber exposes GetCarriedLootBag"), RobberClass->FindFunctionByName(TEXT("GetCarriedLootBag")));
	TestNotNull(TEXT("Robber exposes SetCarriedLootBag"), RobberClass->FindFunctionByName(TEXT("SetCarriedLootBag")));
	TestNotNull(TEXT("Robber exposes DepositCarriedLoot"), RobberClass->FindFunctionByName(TEXT("DepositCarriedLoot")));

	AHeistsLootBag* LootBagCDO = GetMutableDefault<AHeistsLootBag>();
	TestNotNull(TEXT("AHeistsLootBag CDO exists"), LootBagCDO);
	TestTrue(TEXT("Loot bag implements interactable"), LootBagCDO && LootBagCDO->GetClass()->ImplementsInterface(UHeistsInteractable::StaticClass()));

	const TArray<FHeistsInteractionAction> LootBagActions =
		LootBagCDO ? IHeistsInteractable::Execute_GetAvailableInteractionActions(LootBagCDO, nullptr) : TArray<FHeistsInteractionAction>();
	TestTrue(TEXT("Loot bag exposes Pickup"), LootBagActions.ContainsByPredicate([](const FHeistsInteractionAction& Action) { return Action.ActionId == EHeistsInteractionActionId::Pickup; }));
	TestTrue(TEXT("Loot bag exposes Drop"), LootBagActions.ContainsByPredicate([](const FHeistsInteractionAction& Action) { return Action.ActionId == EHeistsInteractionActionId::Drop; }));

	AHeistsLootContainer* LootContainerCDO = GetMutableDefault<AHeistsLootContainer>();
	TestNotNull(TEXT("AHeistsLootContainer CDO exists"), LootContainerCDO);
	TestTrue(TEXT("Loot container exposes Search"), LootContainerCDO && IHeistsInteractable::Execute_GetAvailableInteractionActions(LootContainerCDO, nullptr).ContainsByPredicate([](const FHeistsInteractionAction& Action) { return Action.ActionId == EHeistsInteractionActionId::Search; }));

	AHeistsExtractionZone* ExtractionZoneCDO = GetMutableDefault<AHeistsExtractionZone>();
	TestNotNull(TEXT("AHeistsExtractionZone CDO exists"), ExtractionZoneCDO);
	TestTrue(TEXT("Extraction zone exposes Deposit"), ExtractionZoneCDO && IHeistsInteractable::Execute_GetAvailableInteractionActions(ExtractionZoneCDO, nullptr).ContainsByPredicate([](const FHeistsInteractionAction& Action) { return Action.ActionId == EHeistsInteractionActionId::Deposit; }));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHeistsSharedItemPickupContractTest,
	"Heists.Phase1.SharedItems.PickupContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeistsSharedItemPickupContractTest::RunTest(const FString& Parameters)
{
	AHeistsPickupActor* PickupCDO = GetMutableDefault<AHeistsPickupActor>();
	TestNotNull(TEXT("AHeistsPickupActor CDO exists"), PickupCDO);
	TestTrue(TEXT("Pickup actor implements interactable"), PickupCDO && PickupCDO->GetClass()->ImplementsInterface(UHeistsInteractable::StaticClass()));

	const TArray<FHeistsInteractionAction> PickupActions =
		PickupCDO ? IHeistsInteractable::Execute_GetAvailableInteractionActions(PickupCDO, nullptr) : TArray<FHeistsInteractionAction>();
	TestEqual(TEXT("Pickup actor exposes one action"), PickupActions.Num(), 1);
	if (PickupActions.Num() == 1)
	{
		TestEqual(TEXT("Pickup action id is Pickup"), PickupActions[0].ActionId, EHeistsInteractionActionId::Pickup);
		TestEqual(TEXT("Pickup action color is blue for access items"), PickupActions[0].Color, EHeistsInteractionColor::Blue);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHeistsInteractionControllerContractTest,
	"Heists.Phase1.Interaction.ControllerContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeistsInteractionControllerContractTest::RunTest(const FString& Parameters)
{
	const UClass* ControllerClass = AHeistsPlayerController::StaticClass();
	TestNotNull(TEXT("Controller exposes OpenInteractionRadial"), ControllerClass->FindFunctionByName(TEXT("OpenInteractionRadial")));
	TestNotNull(TEXT("Controller exposes ConfirmInteractionAction"), ControllerClass->FindFunctionByName(TEXT("ConfirmInteractionAction")));
	TestNotNull(TEXT("Controller exposes CancelInteractionRadial"), ControllerClass->FindFunctionByName(TEXT("CancelInteractionRadial")));
	TestNotNull(TEXT("Controller exposes IsInteractionRadialOpen"), ControllerClass->FindFunctionByName(TEXT("IsInteractionRadialOpen")));
	TestNotNull(TEXT("Controller exposes GetRadialActions"), ControllerClass->FindFunctionByName(TEXT("GetRadialActions")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
