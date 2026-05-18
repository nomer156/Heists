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
#include "UI/HeistsHUD.h"
#include "Misc/ConfigCacheIni.h"
#include "GameFramework/TouchInterface.h"
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

	const AHeistsPlayerController* ControllerCDO = GetDefault<AHeistsPlayerController>();
	TestNotNull(TEXT("AHeistsPlayerController CDO exists"), ControllerCDO);
	if (ControllerCDO)
	{
		TestTrue(TEXT("Mobile stick movement is enabled by default"), ControllerCDO->IsMobileStickMovementEnabled());
		TestNotNull(TEXT("Controller CDO has default Enhanced Input mapping context"), ControllerCDO->DefaultMappingContext.Get());
		TestNotNull(TEXT("Controller CDO has default IA_Move for WASD/gamepad/touch stick"), ControllerCDO->IA_Move.Get());
		TestTrue(TEXT("Left-side chat/tasks movement exclusion zones are configured"), ControllerCDO->LeftScreenInputBlockZones.Num() > 0);
	}

	bool bUseMouseForTouch = false;
	GConfig->GetBool(TEXT("/Script/Engine.InputSettings"), TEXT("bUseMouseForTouch"), bUseMouseForTouch, GInputIni);
	TestTrue(TEXT("Mouse simulates touch for editor standalone mobile tests"), bUseMouseForTouch);

	bool bAlwaysShowTouchInterface = false;
	GConfig->GetBool(TEXT("/Script/Engine.InputSettings"), TEXT("bAlwaysShowTouchInterface"), bAlwaysShowTouchInterface, GInputIni);
	TestTrue(TEXT("Virtual joystick is always visible in mobile test windows"), bAlwaysShowTouchInterface);

	FString DefaultTouchInterface;
	GConfig->GetString(TEXT("/Script/Engine.InputSettings"), TEXT("DefaultTouchInterface"), DefaultTouchInterface, GInputIni);
	TestTrue(
		TEXT("Project has a default touch interface asset"),
		!DefaultTouchInterface.IsEmpty() && !DefaultTouchInterface.Equals(TEXT("None")));
	TestNotNull(
		TEXT("Default touch interface resolves to UTouchInterface"),
		LoadObject<UTouchInterface>(nullptr, *DefaultTouchInterface));

	int32 NewWindowWidth = 0;
	int32 NewWindowHeight = 0;
	GConfig->GetInt(TEXT("/Script/UnrealEd.LevelEditorPlaySettings"), TEXT("NewWindowWidth"), NewWindowWidth, GEditorPerProjectIni);
	GConfig->GetInt(TEXT("/Script/UnrealEd.LevelEditorPlaySettings"), TEXT("NewWindowHeight"), NewWindowHeight, GEditorPerProjectIni);
	TestTrue(TEXT("PIE new window is portrait-first for mobile testing"), NewWindowHeight > NewWindowWidth);

	int32 StandaloneWindowWidth = 0;
	int32 StandaloneWindowHeight = 0;
	GConfig->GetInt(TEXT("/Script/UnrealEd.LevelEditorPlaySettings"), TEXT("StandaloneWindowWidth"), StandaloneWindowWidth, GEditorPerProjectIni);
	GConfig->GetInt(TEXT("/Script/UnrealEd.LevelEditorPlaySettings"), TEXT("StandaloneWindowHeight"), StandaloneWindowHeight, GEditorPerProjectIni);
	TestTrue(TEXT("Standalone game window is portrait-first for mobile testing"), StandaloneWindowHeight > StandaloneWindowWidth);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHeistsMobileAdaptiveLayoutContractTest,
	"Heists.Phase0.Input.MobileAdaptiveLayoutContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeistsMobileAdaptiveLayoutContractTest::RunTest(const FString& Parameters)
{
	const UClass* ControllerClass = AHeistsPlayerController::StaticClass();
	TestNotNull(TEXT("Controller exposes current layout mode"), ControllerClass->FindFunctionByName(TEXT("GetCurrentMobileLayoutMode")));
	TestNotNull(TEXT("Controller exposes portrait layout query"), ControllerClass->FindFunctionByName(TEXT("IsPortraitLayoutActive")));
	TestNotNull(TEXT("Controller exposes layout refresh"), ControllerClass->FindFunctionByName(TEXT("RefreshMobileLayoutForViewport")));
	TestNotNull(TEXT("Controller exposes portrait input block zones"), FindFProperty<FArrayProperty>(ControllerClass, TEXT("PortraitScreenInputBlockZones")));
	TestNotNull(TEXT("Controller exposes landscape input block zones"), FindFProperty<FArrayProperty>(ControllerClass, TEXT("LandscapeScreenInputBlockZones")));

	const AHeistsPlayerController* ControllerCDO = GetDefault<AHeistsPlayerController>();
	TestNotNull(TEXT("Controller CDO exists"), ControllerCDO);

	const UClass* HUDClass = AHeistsHUD::StaticClass();
	TestNotNull(TEXT("HUD exposes mobile HUD refresh"), HUDClass->FindFunctionByName(TEXT("RefreshMobileHUD")));
	TestNotNull(TEXT("HUD exposes mobile HUD widget class"), FindFProperty<FClassProperty>(HUDClass, TEXT("MobileHUDWidgetClass")));

	UClass* MobileHUDWidgetClass = FindObject<UClass>(nullptr, TEXT("/Script/Heists.HeistsMobileHUDWidget"));
	TestNotNull(TEXT("Native mobile HUD widget class exists"), MobileHUDWidgetClass);
	if (MobileHUDWidgetClass)
	{
		TestNotNull(TEXT("Mobile HUD widget applies layout mode"), MobileHUDWidgetClass->FindFunctionByName(TEXT("ApplyLayoutMode")));
		TestNotNull(TEXT("Mobile HUD widget exposes layout mode"), MobileHUDWidgetClass->FindFunctionByName(TEXT("GetLayoutMode")));
		TestNotNull(TEXT("Mobile HUD widget exposes required designer widget names"), MobileHUDWidgetClass->FindFunctionByName(TEXT("GetRequiredDesignerWidgetNames")));
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
