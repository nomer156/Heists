// Copyright 2026 Heists. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Interaction/HeistsInteractionTypes.h"

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

#endif // WITH_DEV_AUTOMATION_TESTS
