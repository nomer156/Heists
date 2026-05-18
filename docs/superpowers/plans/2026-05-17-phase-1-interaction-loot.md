# Phase 1 Interaction + Loot Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first replicated heist interaction loop on `MainMap`: radial actions, interaction progress, shared crew items, physical loot bags, and extraction.

**Architecture:** Gameplay logic stays in C++. `AHeistsRobber` delegates interaction scanning and execution to `UHeistsInteractionComponent`; world objects implement `IHeistsInteractable` through focused C++ actors. `AHeistsGameState` owns replicated team-level shared items and collected loot. Debug HUD can be Canvas/C++ first, with stable names/events documented for later UMG visual replacement.

**Tech Stack:** UE 5.6 C++, Gameplay Framework replication, Enhanced Input, UMG/Canvas debug HUD, Automation Tests.

---

## File Map

- Create `Source/Heists/Interaction/HeistsInteractionTypes.h`: enums/structs for action id, task type, color, progress behavior, interaction entry.
- Create `Source/Heists/Interaction/HeistsInteractable.h`: `UINTERFACE` for world interaction targets.
- Create `Source/Heists/Interaction/HeistsInteractionComponent.h/.cpp`: target scanning, action selection, server RPCs, active interaction state.
- Create `Source/Heists/Interaction/HeistsInteractableActorBase.h/.cpp`: base replicated actor for interactable objects.
- Create `Source/Heists/Interaction/HeistsDoorActor.h/.cpp`: open/close/peek/lock/unlock/breach.
- Create `Source/Heists/Interaction/HeistsTerminalActor.h/.cpp`: single-action terminal with hold/timing tasks.
- Create `Source/Heists/Interaction/HeistsPickupActor.h/.cpp`: shared keycard/key/code pickup.
- Create `Source/Heists/Loot/HeistsLootBag.h/.cpp`: physical replicated loot bag.
- Create `Source/Heists/Loot/HeistsLootContainer.h/.cpp`: spawns or grants loot bag.
- Create `Source/Heists/Loot/HeistsExtractionZone.h/.cpp`: deposits carried loot.
- Modify `Source/Heists/Character/HeistsRobber.h/.cpp`: own interaction component, one carried loot bag, replace the current temporary overlap logging path.
- Modify `Source/Heists/Game/HeistsGameState.h/.cpp`: replicated shared crew items.
- Modify `Source/Heists/Player/HeistsPlayerController.h/.cpp`: action button/radial request path.
- Modify `Source/Heists/Character/HeistsCharacterBase.h/.cpp` and/or camera component setup: adaptive mobile camera drag rotation path.
- Modify `Source/Heists/UI/HeistsHUD.h/.cpp`: debug target/action/progress/shared item/loot display.
- Modify `Source/Heists/Tests/HeistsPhase0ArchitectureTests.cpp` or create `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp`: automation coverage.
- Create `.codex/create_phase1_assets.py`: optional editor script to place primitive Phase 1 actors in `MainMap` after C++ compiles.
- Update `ГДД.md`, `index.html`, `AGENTS.md`, `PROJECT_CONTEXT.md` after implementation milestones.

---

### Task 1: Interaction Data Types And Tests

**Files:**
- Create: `Source/Heists/Interaction/HeistsInteractionTypes.h`
- Create: `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp`

- [ ] **Step 1: Write failing automation tests**

Create `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp` with tests that require:

```cpp
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

#endif
```

- [ ] **Step 2: Run build and verify it fails**

Run:

```powershell
F:/UE5/UE_5.6/Engine/Build/BatchFiles/Build.bat HeistsEditor Win64 Development -Project="F:/UE5/Projects/Heists/Heists.uproject" -WaitMutex -NoHotReload
```

Expected: compile fails because `Interaction/HeistsInteractionTypes.h` does not exist.

- [ ] **Step 3: Implement interaction data types**

Create `Source/Heists/Interaction/HeistsInteractionTypes.h`:

```cpp
// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HeistsInteractionTypes.generated.h"

UENUM(BlueprintType)
enum class EHeistsInteractionActionId : uint8
{
	None,
	Open,
	Close,
	Peek,
	Lock,
	Unlock,
	Breach,
	Hack,
	Activate,
	ScanFingerprint,
	TimingInput,
	Pickup,
	Drop,
	Deposit,
	Search,
	Disable,
	Inspect
};

UENUM(BlueprintType)
enum class EHeistsInteractionTaskType : uint8
{
	Instant,
	HoldProgress,
	TimingTap,
	Fingerprint,
	CodeMatch,
	Wiring
};

UENUM(BlueprintType)
enum class EHeistsInteractionColor : uint8
{
	Green,
	Yellow,
	Red,
	Blue,
	Gray
};

UENUM(BlueprintType)
enum class EHeistsInteractionProgressBehavior : uint8
{
	ResetOnCancel,
	PreserveOnCancel,
	ResetOnFailure
};

USTRUCT(BlueprintType)
struct HEISTS_API FHeistsInteractionAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	EHeistsInteractionActionId ActionId = EHeistsInteractionActionId::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	EHeistsInteractionTaskType TaskType = EHeistsInteractionTaskType::Instant;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	EHeistsInteractionColor Color = EHeistsInteractionColor::Gray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction", meta = (ClampMin = "0.0"))
	float Duration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	EHeistsInteractionProgressBehavior ProgressBehavior = EHeistsInteractionProgressBehavior::ResetOnCancel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	FGameplayTag RequiredSharedItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	bool bRequiresSharedItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction")
	bool bIsEnabled = true;
};
```

- [ ] **Step 4: Run build and automation**

Run build command above, then:

```powershell
F:/UE5/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe "F:/UE5/Projects/Heists/Heists.uproject" -NullRHI -Unattended -NoSplash -NoSound -ExecCmds="Automation RunTests Heists.Phase1.Interaction.ActionDefaults; Quit" -abslog="F:/UE5/Projects/Heists/Saved/Logs/Heists_Phase1_ActionDefaults.log"
```

Expected: build succeeds and test passes.

- [ ] **Step 5: Commit**

```powershell
git add Source/Heists/Interaction/HeistsInteractionTypes.h Source/Heists/Tests/HeistsPhase1InteractionTests.cpp
git commit -m "feat: add interaction action types"
```

---

### Task 2: Shared Crew Items In GameState

**Files:**
- Modify: `Source/Heists/Game/HeistsGameState.h`
- Modify: `Source/Heists/Game/HeistsGameState.cpp`
- Modify: `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp`

- [ ] **Step 1: Add failing tests for shared items**

Append to `HeistsPhase1InteractionTests.cpp`:

```cpp
#include "Game/HeistsGameState.h"
#include "GameplayTagsManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHeistsSharedCrewItemsTest,
	"Heists.Phase1.SharedItems.GameStateContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeistsSharedCrewItemsTest::RunTest(const FString& Parameters)
{
	const UClass* GameStateClass = AHeistsGameState::StaticClass();
	TestNotNull(TEXT("GameState exposes HasSharedCrewItem"), GameStateClass->FindFunctionByName(TEXT("HasSharedCrewItem")));
	TestNotNull(TEXT("GameState exposes AddSharedCrewItem"), GameStateClass->FindFunctionByName(TEXT("AddSharedCrewItem")));
	TestNotNull(TEXT("GameState exposes GetSharedCrewItems"), GameStateClass->FindFunctionByName(TEXT("GetSharedCrewItems")));
	return true;
}
```

- [ ] **Step 2: Run build and verify it fails**

Expected: test compile fails until functions are declared.

- [ ] **Step 3: Implement replicated shared item API**

Add to `AHeistsGameState`:

```cpp
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SharedCrewItems, Category = "Heists|SharedItems")
FGameplayTagContainer SharedCrewItems;

UFUNCTION(BlueprintCallable, Category = "Heists|SharedItems")
bool HasSharedCrewItem(FGameplayTag ItemTag) const;

UFUNCTION(BlueprintCallable, Category = "Heists|SharedItems")
void AddSharedCrewItem(FGameplayTag ItemTag);

UFUNCTION(BlueprintCallable, Category = "Heists|SharedItems")
FGameplayTagContainer GetSharedCrewItems() const { return SharedCrewItems; }

UFUNCTION()
void OnRep_SharedCrewItems();

UFUNCTION(BlueprintImplementableEvent, Category = "Heists|SharedItems")
void BP_OnSharedCrewItemsChanged();
```

In `.cpp`, add `DOREPLIFETIME(AHeistsGameState, SharedCrewItems);`, implement authority guard in `AddSharedCrewItem`, and call `BP_OnSharedCrewItemsChanged()` in `OnRep_SharedCrewItems`.

- [ ] **Step 4: Run build and tests**

Run `Build.bat`, then `Automation RunTests Heists.Phase1.SharedItems`.

Expected: pass.

- [ ] **Step 5: Commit**

```powershell
git add Source/Heists/Game/HeistsGameState.* Source/Heists/Tests/HeistsPhase1InteractionTests.cpp
git commit -m "feat: replicate shared crew items"
```

---

### Task 3: Interactable Interface And Component

**Files:**
- Create: `Source/Heists/Interaction/HeistsInteractable.h`
- Create: `Source/Heists/Interaction/HeistsInteractionComponent.h/.cpp`
- Modify: `Source/Heists/Character/HeistsRobber.h/.cpp`
- Modify: `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp`

- [ ] **Step 1: Add failing component/interface tests**

Add tests that check `UHeistsInteractionComponent`, `IHeistsInteractable`, and `AHeistsRobber` has an interaction component.

- [ ] **Step 2: Implement `IHeistsInteractable`**

Required methods:

```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Heists|Interaction")
TArray<FHeistsInteractionAction> GetAvailableInteractionActions(AActor* Interactor) const;

UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Heists|Interaction")
bool CanInteract(AActor* Interactor, EHeistsInteractionActionId ActionId) const;

UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Heists|Interaction")
void BeginInteract(AActor* Interactor, EHeistsInteractionActionId ActionId);

UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Heists|Interaction")
void CancelInteract(AActor* Interactor);

UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Heists|Interaction")
void CommitInteract(AActor* Interactor, EHeistsInteractionActionId ActionId);
```

- [ ] **Step 3: Implement `UHeistsInteractionComponent`**

Core API:

```cpp
UFUNCTION(BlueprintCallable, Category = "Heists|Interaction")
AActor* FindBestInteractable() const;

UFUNCTION(BlueprintCallable, Category = "Heists|Interaction")
void RequestPrimaryInteraction();

UFUNCTION(Server, Reliable, WithValidation)
void Server_RequestInteraction(AActor* Target, EHeistsInteractionActionId ActionId);

UFUNCTION(Server, Reliable)
void Server_CancelInteraction();
```

Scan radius defaults to 250 uu. Use overlap by `ECC_WorldDynamic` and `ECC_Pawn` compatible object types, then filter by interface and distance.

- [ ] **Step 4: Attach component to `AHeistsRobber`**

Add `TObjectPtr<UHeistsInteractionComponent> InteractionComponent;` as a visible subobject. `TryInteract()` calls `InteractionComponent->RequestPrimaryInteraction()`.

- [ ] **Step 5: Build/test/commit**

Run build and `Automation RunTests Heists.Phase1.Interaction`, then commit:

```powershell
git add Source/Heists/Interaction Source/Heists/Character/HeistsRobber.* Source/Heists/Tests/HeistsPhase1InteractionTests.cpp
git commit -m "feat: add interaction component"
```

---

### Task 4: Interactable Actor Base, Door, Terminal

**Files:**
- Create: `Source/Heists/Interaction/HeistsInteractableActorBase.h/.cpp`
- Create: `Source/Heists/Interaction/HeistsDoorActor.h/.cpp`
- Create: `Source/Heists/Interaction/HeistsTerminalActor.h/.cpp`
- Modify: `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp`

- [ ] **Step 1: Write tests for default actions**

Door should expose `Open`, `Close`, `Peek`, `Lock`, `Unlock`, `Breach`. Terminal should expose one configured default action.

- [ ] **Step 2: Implement base actor**

Base actor stores:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Heists|Interaction")
TArray<FHeistsInteractionAction> Actions;

UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ActiveInteractor, Category = "Heists|Interaction")
TObjectPtr<AActor> ActiveInteractor;

UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Progress, Category = "Heists|Interaction")
float InteractionProgress;
```

It implements busy state, progress timer, cancel, commit, and shared-item checks.

- [ ] **Step 3: Implement door**

Replicate `bIsOpen`, `bIsLocked`, `bIsPeeked`. `Breach` unlocks/opens and logs a noisy action stub.

- [ ] **Step 4: Implement terminal**

Supports single configured action. Defaults to `Hack`, Yellow, `HoldProgress`, duration 3 seconds, preserve progress.

- [ ] **Step 5: Build/test/commit**

Commit message:

```powershell
git commit -m "feat: add door and terminal interactions"
```

---

### Task 5: Loot Bag, Loot Container, Extraction Zone

**Files:**
- Create: `Source/Heists/Loot/HeistsLootBag.h/.cpp`
- Create: `Source/Heists/Loot/HeistsLootContainer.h/.cpp`
- Create: `Source/Heists/Loot/HeistsExtractionZone.h/.cpp`
- Modify: `Source/Heists/Character/HeistsRobber.h/.cpp`
- Modify: `Source/Heists/Game/HeistsGameState.h/.cpp`
- Modify: `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp`

- [ ] **Step 1: Write tests for loot carry contract**

Test required functions/properties exist: `GetCarriedLootBag`, `CanCarryLootBag`, `SetCarriedLootBag`, `DepositCarriedLoot`.

- [ ] **Step 2: Implement loot bag**

Replicated actor with `LootValue`, `Weight`, `bIsCarried`, `Carrier`. Implements interactable actions `Pickup` and `Drop`.

- [ ] **Step 3: Implement robber carry state**

`AHeistsRobber` stores replicated `CarriedLootBag`. Carrying applies `MoveSpeedMultiplier = 0.75` by adjusting character movement max walk speed from a stored base speed.

- [ ] **Step 4: Implement loot container**

Interactable actor with `Search` or `Open`, creates a loot bag at a spawn transform.

- [ ] **Step 5: Implement extraction zone**

Interactable actor action `Deposit`; if interactor carries bag, adds value to `AHeistsGameState::AddLoot`, destroys or hides bag, clears carry state.

- [ ] **Step 6: Build/test/commit**

Commit message:

```powershell
git commit -m "feat: add physical loot bag flow"
```

---

### Task 6: Pickup Actors And Shared Item Requirements

**Files:**
- Create: `Source/Heists/Interaction/HeistsPickupActor.h/.cpp`
- Modify: `Source/Heists/Interaction/HeistsInteractableActorBase.cpp`
- Modify: `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp`

- [ ] **Step 1: Write tests**

Pickup actor must expose `Pickup` and a shared item tag. Base interactable must deny actions when required shared item is missing.

- [ ] **Step 2: Implement pickup actor**

On commit, pickup calls `AHeistsGameState::AddSharedCrewItem(ItemTag)` and hides/destroys itself.

- [ ] **Step 3: Implement shared item gate**

In base actor `CanInteract`, if action requires a shared item and GameState does not have it, return false.

- [ ] **Step 4: Build/test/commit**

Commit message:

```powershell
git commit -m "feat: add shared item pickups"
```

---

### Task 7: PlayerController Action Path And Debug HUD

**Files:**
- Modify: `Source/Heists/Player/HeistsPlayerController.h/.cpp`
- Modify: `Source/Heists/UI/HeistsHUD.h/.cpp`
- Modify: `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp`

- [ ] **Step 1: Add controller contract tests**

Verify controller exposes `OpenInteractionRadial`, `ConfirmInteractionAction`, `CancelInteractionRadial`.

- [ ] **Step 2: Implement controller API**

Controller asks pawn interaction component for current target/actions. If one action, request it. If multiple actions, cache target/actions and tell HUD to draw radial.

- [ ] **Step 3: Implement HUD debug drawing**

Use `AHeistsHUD::DrawHUD`:

- current target name;
- action list/radial sectors as colored debug wedges or simple colored labels;
- progress bar;
- shared items;
- carried bag.

The HUD remains code-driven debug UI until UMG visuals are manually polished.

- [ ] **Step 4: Build/test/commit**

Commit message:

```powershell
git commit -m "feat: add radial action debug hud"
```

---

### Task 8: MainMap Phase 1 Prototype Actors

**Files:**
- Create: `.codex/create_phase1_assets.py`
- Binary changes: `Content/Maps/MainMap.umap`
- Optional BP visual defaults under `Content/Heists/Blueprints/Interaction`

- [ ] **Step 1: Create editor script**

Script loads `MainMap`, places primitive actors:

- `AHeistsDoorActor`
- `AHeistsTerminalActor`
- `AHeistsPickupActor` for blue keycard
- `AHeistsLootContainer`
- `AHeistsExtractionZone`

- [ ] **Step 2: Run editor script**

Run:

```powershell
F:/UE5/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe "F:/UE5/Projects/Heists/Heists.uproject" -NullRHI -Unattended -NoSplash -NoSound -ExecutePythonScript="F:/UE5/Projects/Heists/.codex/create_phase1_assets.py" -abslog="F:/UE5/Projects/Heists/Saved/Logs/CreatePhase1Assets.log"
```

- [ ] **Step 3: Validate map loads**

Run a commandlet/editor load with `MainMap` and check log for map load errors.

- [ ] **Step 4: Commit**

Commit message:

```powershell
git add .codex/create_phase1_assets.py Content/Maps/MainMap.umap
git commit -m "chore: add phase 1 prototype actors"
```

---

### Task 9: Verification And Living Docs

**Files:**
- Modify: `ГДД.md`
- Modify: `index.html`
- Modify: `AGENTS.md`
- Modify: `PROJECT_CONTEXT.md`

- [ ] **Step 1: Full build**

Run:

```powershell
F:/UE5/UE_5.6/Engine/Build/BatchFiles/Build.bat HeistsEditor Win64 Development -Project="F:/UE5/Projects/Heists/Heists.uproject" -WaitMutex -NoHotReload
```

Expected: success.

- [ ] **Step 2: Automation**

Run:

```powershell
F:/UE5/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe "F:/UE5/Projects/Heists/Heists.uproject" -NullRHI -Unattended -NoSplash -NoSound -ExecCmds="Automation RunTests Heists.Phase1; Quit" -abslog="F:/UE5/Projects/Heists/Saved/Logs/Heists_Phase1_Final.log"
```

Expected: all Phase 1 automation tests pass.

- [ ] **Step 3: Manual PIE smoke instructions**

Document for the user:

- Standalone Game;
- Listen Server + 2 Clients;
- verify WASD/touch joystick movement;
- interact with door radial;
- run terminal action;
- pick up keycard and see shared item;
- pick up/drop/deposit loot bag.

- [ ] **Step 4: Update living docs**

Mark Phase 1 implementation progress and any limitations.

- [ ] **Step 5: Commit and push**

```powershell
git add -A
git commit -m "docs: record phase 1 implementation status"
git push origin main
```

---

### Task 10: Right-Side Camera Drag Input

**Status 2026-05-18:** Implemented in `AHeistsPlayerController` with `bEnableRightSideCameraDrag`, `CameraDragYawSpeed`, `IsScreenPositionCameraDragZone`, mouse-as-touch fallback, and right-side UI block zones. Verified by `Heists.Phase1.Interaction.UsabilityContract`, full `Heists.Phase1`, and `Heists.Phase0`.

**Files:**
- Modify: `Source/Heists/Player/HeistsPlayerController.h/.cpp`
- Modify if needed: `Source/Heists/Character/HeistsCharacterBase.h/.cpp`
- Modify if needed: input assets/defaults under `Content/Input` and `Config/DefaultInput.ini`
- Modify tests: `Source/Heists/Tests/HeistsPhase0ArchitectureTests.cpp` or `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp`

- [ ] **Step 1: Add input contract tests**

Controller/character must expose a stable camera-drag path that can be tested without final UI art. Test expected functions/properties exist for enabling adaptive camera drag and keeping it separate from interaction/radial input.

- [ ] **Step 2: Implement touch/mouse camera drag gate**

Touch/mouse drag that starts on the free right half of the screen rotates the isometric camera. Touches that start over right-side UI, radial sectors, action buttons, progress widgets, chat, or task UI are consumed by UI and must not rotate the camera.

- [ ] **Step 3: Keep movement contract unchanged**

Left-side joystick/WASD movement remains primary movement input. Right-side camera drag must not change movement vectors or interfere with `E`/`1-6` editor debug interaction fallback.

- [ ] **Step 4: Verify**

Run build and automation, then manual Standalone/PIE smoke:

- left drag moves;
- right free drag rotates camera;
- right action/radial UI tap does not rotate camera;
- mouse-as-touch fallback works in editor windows.

---

### Task 11: Interaction Usability Pass

**Status 2026-05-18:** Implemented as a narrow Phase 1 usability pass.

**Files:**
- Modified: `Source/Heists/Interaction/HeistsInteractionComponent.h/.cpp`
- Modified: `Source/Heists/Interaction/HeistsInteractableActorBase.h/.cpp`
- Modified: `Source/Heists/Player/HeistsPlayerController.h/.cpp`
- Modified: `Source/Heists/UI/HeistsHUD.h/.cpp`
- Created: `Source/Heists/UI/HeistsInteractionMenuWidget.h/.cpp`
- Created then moved: `Content/UI/WBP_InteractionMenu.uasset`
- Created: `.codex/create_interaction_menu_widget.py`
- Modified: `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp`

- [x] **Step 1: Local interactable focus hint**

`UHeistsInteractionComponent` scans on the locally controlled pawn and calls `AHeistsInteractableActorBase::SetLocallyFocused`. Prototype actors turn green and enable custom depth while focused. This is a local UI hint, not replicated gameplay state.

- [x] **Step 2: Temporary button menu**

`UHeistsInteractionMenuWidget` provides a native UMG-backed action list with buttons for up to six actions. `WBP_InteractionMenu` is a Blueprint child for manual visual polish. The existing `E` + `1-6` debug path remains.

- [x] **Step 3: Drop carried loot**

`AHeistsPlayerController::DropCarriedLoot` exposes the drop action for UI and binds `G` as editor/dev fallback. `AHeistsRobber::DropLoot` remains server-safe.

- [x] **Step 4: Verification**

Build passed. After Phase 1.5 updates, `Heists.Phase1` passed 9/9 and `Heists.Phase0` passed 3/3.

---

### Task 12: Phase 1.5 Mobile Portrait + Tactical Context Foundation

**Status 2026-05-18:** Implemented as the next large step before deeper stealth/AI. Portrait is now the primary mobile layout; landscape remains fallback when the viewport is wider than tall.

**Files:**
- Modified: `Source/Heists/Player/HeistsPlayerController.h/.cpp`
- Modified: `Source/Heists/UI/HeistsHUD.h/.cpp`
- Created: `Source/Heists/UI/HeistsMobileLayoutTypes.h`
- Created: `Source/Heists/UI/HeistsMobileHUDWidget.h/.cpp`
- Created: `Source/Heists/Character/HeistsCoverComponent.h/.cpp`
- Modified: `Source/Heists/Character/HeistsRobber.h/.cpp`
- Modified: `Source/Heists/Tests/HeistsPhase0ArchitectureTests.cpp`
- Modified: `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp`
- Moved: `Content/Heists/Blueprints/UI/WBP_InteractionMenu.uasset` -> `Content/UI/WBP_InteractionMenu.uasset`
- Created: `Content/UI/WBP_MobileHUD.uasset`
- Modified: `Config/DefaultEditorPerProjectUserSettings.ini`
- Created: `.codex/configure_phase15_ui_assets.py`

- [x] **Step 1: Portrait-first adaptive input contract**

`AHeistsPlayerController` detects viewport shape and exposes `RefreshMobileLayoutForViewport`, `GetCurrentMobileLayoutMode`, and `IsPortraitLayoutActive`. Portrait block zones reserve top objectives/status and bottom controls. Landscape keeps the previous left/right split.

- [x] **Step 2: Adaptive mobile HUD contract**

`UHeistsMobileHUDWidget` is the native parent for `WBP_MobileHUD`. If the Blueprint is empty, the native widget builds a simple debug layout with Interact/Drop buttons. Stable designer names for manual visual polish are:
`Panel_Objectives`, `Panel_QuickCommands`, `Panel_PortraitRoot`, `Panel_LandscapeRoot`, `Button_Interact`, `Button_DropBag`, `ActionList`.

- [x] **Step 3: UI assets moved to root UI folder**

Runtime UI Blueprints now live under `/Content/UI`. `AHeistsHUD` resolves `/Game/UI/WBP_InteractionMenu` and `/Game/UI/WBP_MobileHUD`.

- [x] **Step 4: Cover foundation**

`UHeistsCoverComponent` is attached to `AHeistsRobber` and replicates cover state, cover actor, and cover normal. Current implementation is a foundation only: actual wall-stick movement, animation offset, peek, and cover-specific camera tuning are the next tactical context tasks.

- [x] **Step 5: Verification**

Build passed. `Heists.Phase0` passed 3/3. `Heists.Phase1` passed 9/9.

**Deferred to Phase 2 user batch:** final/manual mesh work, character animation polish, object visuals, UI skinning, and map readability polish should be handled together at the start of Phase 2 instead of interrupting each small C++ step.

---

## Self-Review

- Spec coverage: interaction model, radial UX, shared items, physical loot, task types, debug HUD, and network smoke all have tasks.
- Scope control: main menu, hub, planning, escape, full stealth AI, full role trees, full inventory, and dedicated server packaging remain out of scope.
- Type consistency: tasks consistently use `FHeistsInteractionAction`, `EHeistsInteractionActionId`, `UHeistsInteractionComponent`, and `IHeistsInteractable`.
- Risk: UMG radial may be deferred to Canvas debug HUD if Widget Blueprint generation becomes unstable; gameplay API still keeps stable functions/events for later visual replacement.
