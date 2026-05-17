// Copyright 2026 Heists. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/HeistsAttributeSet.h"
#include "Character/HeistsCharacterBase.h"
#include "Character/HeistsRobber.h"
#include "Player/HeistsPlayerController.h"
#include "Player/HeistsPlayerState.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHeistsPlayerStateOwnsGASTest,
	"Heists.Phase0.GAS.PlayerStateOwnsAbilitySystem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeistsPlayerStateOwnsGASTest::RunTest(const FString& Parameters)
{
	const AHeistsPlayerState* PlayerStateCDO = GetDefault<AHeistsPlayerState>();
	TestNotNull(TEXT("AHeistsPlayerState CDO exists"), PlayerStateCDO);

	const IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(PlayerStateCDO);
	TestNotNull(TEXT("AHeistsPlayerState implements IAbilitySystemInterface"), AbilityInterface);

	if (AbilityInterface)
	{
		const UAbilitySystemComponent* AbilitySystemComponent = AbilityInterface->GetAbilitySystemComponent();
		TestNotNull(TEXT("AHeistsPlayerState owns the replicated AbilitySystemComponent"), AbilitySystemComponent);

		bool bHasHeistsAttributeSet = false;
		if (AbilitySystemComponent)
		{
			for (const UAttributeSet* AttributeSet : AbilitySystemComponent->GetSpawnedAttributes())
			{
				if (Cast<UHeistsAttributeSet>(AttributeSet))
				{
					bHasHeistsAttributeSet = true;
					break;
				}
			}
		}

		TestTrue(TEXT("AHeistsPlayerState owns UHeistsAttributeSet through ASC"), bHasHeistsAttributeSet);
	}

	const AHeistsCharacterBase* CharacterCDO = GetDefault<AHeistsRobber>();
	TestNotNull(TEXT("AHeistsRobber CDO exists"), CharacterCDO);
	TestNull(TEXT("Characters use PlayerState ASC and do not own ASC subobjects"), CharacterCDO->GetAbilitySystemComponent());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHeistsMobileInputContractTest,
	"Heists.Phase0.Input.MobileLandscapeContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeistsMobileInputContractTest::RunTest(const FString& Parameters)
{
	const UClass* ControllerClass = AHeistsPlayerController::StaticClass();
	TestNotNull(TEXT("AHeistsPlayerController class exists"), ControllerClass);

	TestNotNull(
		TEXT("Controller exposes bUseMobileStickMovement"),
		FindFProperty<FBoolProperty>(ControllerClass, TEXT("bUseMobileStickMovement")));

	TestNotNull(
		TEXT("Controller exposes IA_Move for left-half virtual joystick movement"),
		FindFProperty<FObjectProperty>(ControllerClass, TEXT("IA_Move")));

	TestNotNull(
		TEXT("Controller exposes IA_ClickMove as PC/editor fallback"),
		FindFProperty<FObjectProperty>(ControllerClass, TEXT("IA_ClickMove")));

	TestNotNull(
		TEXT("Controller exposes left input exclusion zones for chat/tasks"),
		FindFProperty<FArrayProperty>(ControllerClass, TEXT("LeftScreenInputBlockZones")));

	TestNotNull(
		TEXT("Controller exposes server-safe ability slot action stub"),
		ControllerClass->FindFunctionByName(TEXT("Server_TriggerAbilitySlot")));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
