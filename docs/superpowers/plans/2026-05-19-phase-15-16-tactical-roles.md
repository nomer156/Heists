# Phase 1.5 + 1.6 Tactical Roles Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the next large prototype layer: tactical context, cover/peek, auto-context action hints, four role gameplay identities, timing mini-task hooks, and one mission-flow contract on `MainMap`.

**Architecture:** Gameplay remains C++ and server-authoritative. Roles are replicated state with small tuning data, tactical context extends the existing robber/interaction/cover components, and HUD only reads stable native APIs. Blueprint remains visual/defaults only.

**Tech Stack:** UE 5.6 C++, Gameplay Framework replication, Enhanced Input, UMG debug HUD, Automation Tests.

---

## File Map

- Create `Source/Heists/Roles/HeistsRoleTypes.h`: role enum and role tuning struct.
- Modify `Source/Heists/Player/HeistsPlayerState.h/.cpp`: replicated crew role API.
- Modify `Source/Heists/Character/HeistsRobber.h/.cpp`: role getter/tuning bridge and cover input API.
- Modify `Source/Heists/Character/HeistsCoverComponent.h/.cpp`: enter/exit/peek state, server-safe cover requests.
- Modify `Source/Heists/Interaction/HeistsInteractionTypes.h`: contextual action result and timing tap task data.
- Modify `Source/Heists/Interaction/HeistsInteractionComponent.h/.cpp`: best contextual action and role-aware action duration helper.
- Modify `Source/Heists/Game/HeistsGameState.h/.cpp`: prototype mission result/state contract.
- Modify `Source/Heists/UI/HeistsHUD.h/.cpp`: debug display for role, cover, contextual action, mission result.
- Modify `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp`: Phase 1.5/1.6 contract tests.
- Update `PROJECT_CONTEXT.md`, `ГДД.md`, `AGENTS.md`, `index.html` after implementation.

---

### Task 1: Role Types And Replicated Role Contract

**Files:**
- Create: `Source/Heists/Roles/HeistsRoleTypes.h`
- Modify: `Source/Heists/Player/HeistsPlayerState.h/.cpp`
- Modify: `Source/Heists/Character/HeistsRobber.h/.cpp`
- Modify: `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp`

- [ ] **Step 1: Write failing role contract test**

Append a test named `Heists.Phase1.Roles.Contract` requiring:

```cpp
#include "Roles/HeistsRoleTypes.h"
#include "Player/HeistsPlayerState.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHeistsRolesContractTest,
	"Heists.Phase1.Roles.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeistsRolesContractTest::RunTest(const FString& Parameters)
{
	FHeistsRoleTuning HackerTuning;
	HackerTuning.Role = EHeistsCrewRole::Hacker;
	HackerTuning.HackDurationMultiplier = 0.65f;
	TestEqual(TEXT("Role tuning stores Hacker role"), HackerTuning.Role, EHeistsCrewRole::Hacker);
	TestTrue(TEXT("Hacker tuning speeds up hacks"), HackerTuning.HackDurationMultiplier < 1.f);

	const UClass* PlayerStateClass = AHeistsPlayerState::StaticClass();
	TestNotNull(TEXT("PlayerState exposes GetCrewRole"), PlayerStateClass->FindFunctionByName(TEXT("GetCrewRole")));
	TestNotNull(TEXT("PlayerState exposes SetCrewRole"), PlayerStateClass->FindFunctionByName(TEXT("SetCrewRole")));
	TestNotNull(TEXT("PlayerState exposes OnRep_CrewRole"), PlayerStateClass->FindFunctionByName(TEXT("OnRep_CrewRole")));
	TestNotNull(TEXT("PlayerState replicates CrewRole"), FindFProperty<FByteProperty>(PlayerStateClass, TEXT("CrewRole")));

	const UClass* RobberClass = AHeistsRobber::StaticClass();
	TestNotNull(TEXT("Robber exposes GetCrewRole"), RobberClass->FindFunctionByName(TEXT("GetCrewRole")));
	TestNotNull(TEXT("Robber exposes GetRoleTuning"), RobberClass->FindFunctionByName(TEXT("GetRoleTuning")));
	return true;
}
```

- [ ] **Step 2: Run build and verify RED**

Run:

```powershell
F:/UE5/UE_5.6/Engine/Build/BatchFiles/Build.bat HeistsEditor Win64 Development -Project="F:/UE5/Projects/Heists/Heists.uproject" -WaitMutex -NoHotReload
```

Expected: compile fails because `Roles/HeistsRoleTypes.h` and role APIs do not exist.

- [ ] **Step 3: Implement role types**

Create `Source/Heists/Roles/HeistsRoleTypes.h`:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "HeistsRoleTypes.generated.h"

UENUM(BlueprintType)
enum class EHeistsCrewRole : uint8
{
	None,
	Coordinator,
	Hacker,
	Breaker,
	Scout,
	Driver
};

USTRUCT(BlueprintType)
struct HEISTS_API FHeistsRoleTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Roles")
	EHeistsCrewRole Role = EHeistsCrewRole::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Roles", meta = (ClampMin = "0.1"))
	float HackDurationMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Roles", meta = (ClampMin = "0.1"))
	float ForceDurationMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Roles", meta = (ClampMin = "0.1"))
	float QuietInteractionMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Roles", meta = (ClampMin = "0.0"))
	float ContextScanRangeBonus = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Roles", meta = (ClampMin = "0.0"))
	float TeamInfoRangeBonus = 0.f;
};
```

- [ ] **Step 4: Add replicated role state to PlayerState**

Add to `AHeistsPlayerState`:

```cpp
#include "Roles/HeistsRoleTypes.h"

UFUNCTION(BlueprintCallable, Category = "Heists|Roles")
EHeistsCrewRole GetCrewRole() const { return CrewRole; }

UFUNCTION(BlueprintCallable, Category = "Heists|Roles")
void SetCrewRole(EHeistsCrewRole NewRole);

UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CrewRole, Category = "Heists|Roles")
EHeistsCrewRole CrewRole = EHeistsCrewRole::None;

UFUNCTION()
void OnRep_CrewRole();

UFUNCTION(BlueprintImplementableEvent, Category = "Heists|Roles")
void BP_OnCrewRoleChanged(EHeistsCrewRole NewRole);
```

Implementation requirements:

```cpp
void AHeistsPlayerState::SetCrewRole(EHeistsCrewRole NewRole)
{
	if (!HasAuthority())
	{
		return;
	}
	CrewRole = NewRole;
	PlayerRole = StaticEnum<EHeistsCrewRole>()->GetNameStringByValue(static_cast<int64>(CrewRole));
	BP_OnCrewRoleChanged(CrewRole);
}

void AHeistsPlayerState::OnRep_CrewRole()
{
	BP_OnCrewRoleChanged(CrewRole);
}

DOREPLIFETIME(AHeistsPlayerState, CrewRole);
```

- [ ] **Step 5: Add role bridge to Robber**

Add to `AHeistsRobber`:

```cpp
UFUNCTION(BlueprintPure, Category = "Heists|Robber|Role")
EHeistsCrewRole GetCrewRole() const;

UFUNCTION(BlueprintPure, Category = "Heists|Robber|Role")
FHeistsRoleTuning GetRoleTuning() const;
```

Implementation returns PlayerState role if available, else maps legacy `RoleType`. Tuning defaults:

```cpp
Coordinator: TeamInfoRangeBonus = 400.f
Hacker: HackDurationMultiplier = 0.65f
Breaker: ForceDurationMultiplier = 0.65f
Scout: QuietInteractionMultiplier = 0.85f, ContextScanRangeBonus = 150.f
Driver: no robber bonus in Phase 1
```

- [ ] **Step 6: Run build and targeted test**

Run build, then:

```powershell
F:/UE5/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe "F:/UE5/Projects/Heists/Heists.uproject" -NullRHI -Unattended -NoSplash -NoSound -DDC-ForceMemoryCache -ExecCmds="Automation RunTests Heists.Phase1.Roles.Contract; Quit" -abslog="F:/UE5/Projects/Heists/Saved/Logs/Heists_Phase1_Roles.log"
```

Expected: test passes.

---

### Task 2: Tactical Cover, Wall-Stick, And Peek Contract

**Files:**
- Modify: `Source/Heists/Character/HeistsCoverComponent.h/.cpp`
- Modify: `Source/Heists/Character/HeistsRobber.h/.cpp`
- Modify: `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp`

- [ ] **Step 1: Write failing cover contract test**

Add test `Heists.Phase1.Cover.TacticalContract` requiring:

```cpp
UClass* CoverComponentClass = FindObject<UClass>(nullptr, TEXT("/Script/Heists.HeistsCoverComponent"));
TestNotNull(TEXT("Cover component exposes RequestEnterCover"), CoverComponentClass->FindFunctionByName(TEXT("RequestEnterCover")));
TestNotNull(TEXT("Cover component exposes RequestExitCover"), CoverComponentClass->FindFunctionByName(TEXT("RequestExitCover")));
TestNotNull(TEXT("Cover component exposes RequestTogglePeek"), CoverComponentClass->FindFunctionByName(TEXT("RequestTogglePeek")));
TestNotNull(TEXT("Cover component exposes IsPeeking"), CoverComponentClass->FindFunctionByName(TEXT("IsPeeking")));
TestNotNull(TEXT("Cover component exposes GetCoverStateName"), CoverComponentClass->FindFunctionByName(TEXT("GetCoverStateName")));

const UClass* RobberClass = AHeistsRobber::StaticClass();
TestNotNull(TEXT("Robber exposes RequestEnterCover"), RobberClass->FindFunctionByName(TEXT("RequestEnterCover")));
TestNotNull(TEXT("Robber exposes RequestExitCover"), RobberClass->FindFunctionByName(TEXT("RequestExitCover")));
TestNotNull(TEXT("Robber exposes RequestTogglePeek"), RobberClass->FindFunctionByName(TEXT("RequestTogglePeek")));
```

- [ ] **Step 2: Implement cover state enum and requests**

Add to `HeistsCoverComponent.h`:

```cpp
UENUM(BlueprintType)
enum class EHeistsCoverState : uint8
{
	None,
	InCover,
	Peeking
};

UFUNCTION(BlueprintCallable, Category = "Heists|Cover")
void RequestEnterCover();

UFUNCTION(BlueprintCallable, Category = "Heists|Cover")
void RequestExitCover();

UFUNCTION(BlueprintCallable, Category = "Heists|Cover")
void RequestTogglePeek();

UFUNCTION(Server, Reliable)
void Server_SetCoverState(EHeistsCoverState NewState);

UFUNCTION(BlueprintPure, Category = "Heists|Cover")
bool IsPeeking() const { return CoverState == EHeistsCoverState::Peeking; }

UFUNCTION(BlueprintPure, Category = "Heists|Cover")
FName GetCoverStateName() const;

UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|Cover")
EHeistsCoverState CoverState = EHeistsCoverState::None;
```

Rules:

- `RequestEnterCover` refreshes cover state first and only enters cover if a cover actor is detected.
- `RequestExitCover` always clears state.
- `RequestTogglePeek` toggles `InCover <-> Peeking`.
- Server owns replicated `CoverState`.

- [ ] **Step 3: Add Robber forwarding API**

Add to `AHeistsRobber`:

```cpp
UFUNCTION(BlueprintCallable, Category = "Heists|Robber|Cover")
void RequestEnterCover();

UFUNCTION(BlueprintCallable, Category = "Heists|Robber|Cover")
void RequestExitCover();

UFUNCTION(BlueprintCallable, Category = "Heists|Robber|Cover")
void RequestTogglePeek();
```

Each function checks `CoverComponent` and forwards to it.

- [ ] **Step 4: Run build and cover tests**

Run build and:

```powershell
F:/UE5/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe "F:/UE5/Projects/Heists/Heists.uproject" -NullRHI -Unattended -NoSplash -NoSound -DDC-ForceMemoryCache -ExecCmds="Automation RunTests Heists.Phase1.Cover; Quit" -abslog="F:/UE5/Projects/Heists/Saved/Logs/Heists_Phase1_Cover.log"
```

Expected: cover foundation and tactical contract pass.

---

### Task 3: Auto-Context Actions And Timing Task Hook

**Files:**
- Modify: `Source/Heists/Interaction/HeistsInteractionTypes.h`
- Modify: `Source/Heists/Interaction/HeistsInteractionComponent.h/.cpp`
- Modify: `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp`

- [ ] **Step 1: Write failing interaction task contract test**

Add test `Heists.Phase1.Interaction.ContextAndTimingContract` requiring:

```cpp
FHeistsTimingTapTask TimingTask;
TimingTask.TargetWindowStart = 0.4f;
TimingTask.TargetWindowEnd = 0.6f;
TestTrue(TEXT("Timing task accepts midpoint hit"), TimingTask.IsHit(0.5f));
TestFalse(TEXT("Timing task rejects early hit"), TimingTask.IsHit(0.2f));

const UClass* ComponentClass = UHeistsInteractionComponent::StaticClass();
TestNotNull(TEXT("Component exposes GetBestContextualActionForTarget"), ComponentClass->FindFunctionByName(TEXT("GetBestContextualActionForTarget")));
TestNotNull(TEXT("Component exposes GetRoleAdjustedActionDuration"), ComponentClass->FindFunctionByName(TEXT("GetRoleAdjustedActionDuration")));
```

- [ ] **Step 2: Implement timing task data**

Add to `HeistsInteractionTypes.h`:

```cpp
USTRUCT(BlueprintType)
struct HEISTS_API FHeistsTimingTapTask
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction|Timing")
	float TargetWindowStart = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction|Timing")
	float TargetWindowEnd = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heists|Interaction|Timing")
	float CurrentMarker = 0.f;

	bool IsHit(float NormalizedTime) const
	{
		return NormalizedTime >= TargetWindowStart && NormalizedTime <= TargetWindowEnd;
	}
};
```

- [ ] **Step 3: Implement auto-context helper**

Add to `UHeistsInteractionComponent`:

```cpp
UFUNCTION(BlueprintCallable, Category = "Heists|Interaction")
FHeistsInteractionAction GetBestContextualActionForTarget(AActor* Target) const;

UFUNCTION(BlueprintCallable, Category = "Heists|Interaction")
float GetRoleAdjustedActionDuration(const FHeistsInteractionAction& Action) const;
```

Rules:

- Return first enabled action if only one action exists.
- Prefer `Hack` for hacker, `Breach` for breaker, `Peek/Open/Search` for scout, and `Inspect/Activate` for coordinator when available.
- Fallback to first enabled action.
- Duration multiplier uses `AHeistsRobber::GetRoleTuning`.

- [ ] **Step 4: Run build and interaction tests**

Run build and:

```powershell
F:/UE5/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe "F:/UE5/Projects/Heists/Heists.uproject" -NullRHI -Unattended -NoSplash -NoSound -DDC-ForceMemoryCache -ExecCmds="Automation RunTests Heists.Phase1.Interaction; Quit" -abslog="F:/UE5/Projects/Heists/Saved/Logs/Heists_Phase1_ContextTiming.log"
```

Expected: interaction tests pass.

---

### Task 4: Prototype Mission Flow Contract

**Files:**
- Modify: `Source/Heists/Game/HeistsGameState.h/.cpp`
- Modify: `Source/Heists/UI/HeistsHUD.h/.cpp`
- Modify: `Source/Heists/Tests/HeistsPhase1InteractionTests.cpp`

- [ ] **Step 1: Write failing mission contract test**

Add test `Heists.Phase1.Mission.PrototypeFlowContract` requiring:

```cpp
const UClass* GameStateClass = AHeistsGameState::StaticClass();
TestNotNull(TEXT("GameState exposes StartPrototypeMission"), GameStateClass->FindFunctionByName(TEXT("StartPrototypeMission")));
TestNotNull(TEXT("GameState exposes CompletePrototypeMission"), GameStateClass->FindFunctionByName(TEXT("CompletePrototypeMission")));
TestNotNull(TEXT("GameState exposes FailPrototypeMission"), GameStateClass->FindFunctionByName(TEXT("FailPrototypeMission")));
TestNotNull(TEXT("GameState exposes GetPrototypeMissionResult"), GameStateClass->FindFunctionByName(TEXT("GetPrototypeMissionResult")));
TestNotNull(TEXT("GameState replicates PrototypeMissionResult"), FindFProperty<FStructProperty>(GameStateClass, TEXT("PrototypeMissionResult")));
```

- [ ] **Step 2: Implement mission result data**

Add to `HeistsGameState.h`:

```cpp
USTRUCT(BlueprintType)
struct HEISTS_API FHeistsPrototypeMissionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Heists|Mission")
	bool bMissionActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Heists|Mission")
	bool bMissionCompleted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Heists|Mission")
	bool bMissionFailed = false;

	UPROPERTY(BlueprintReadOnly, Category = "Heists|Mission")
	int32 DeliveredLootValue = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Heists|Mission")
	int32 SharedItemsAcquired = 0;
};
```

Add API:

```cpp
UFUNCTION(BlueprintCallable, Category = "Heists|Mission")
void StartPrototypeMission();

UFUNCTION(BlueprintCallable, Category = "Heists|Mission")
void CompletePrototypeMission();

UFUNCTION(BlueprintCallable, Category = "Heists|Mission")
void FailPrototypeMission();

UFUNCTION(BlueprintCallable, Category = "Heists|Mission")
FHeistsPrototypeMissionResult GetPrototypeMissionResult() const { return PrototypeMissionResult; }

UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PrototypeMissionResult, Category = "Heists|Mission")
FHeistsPrototypeMissionResult PrototypeMissionResult;
```

- [ ] **Step 3: Update HUD debug output**

In `AHeistsHUD::DrawHUD`, add one debug line showing:

```cpp
Mission: Active/Completed/Failed | Loot: X | Shared Items: Y
```

Only read from `AHeistsGameState`; do not store duplicate HUD state.

- [ ] **Step 4: Run build and mission test**

Run build and:

```powershell
F:/UE5/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe "F:/UE5/Projects/Heists/Heists.uproject" -NullRHI -Unattended -NoSplash -NoSound -DDC-ForceMemoryCache -ExecCmds="Automation RunTests Heists.Phase1.Mission; Quit" -abslog="F:/UE5/Projects/Heists/Saved/Logs/Heists_Phase1_Mission.log"
```

Expected: mission contract passes.

---

### Task 5: Full Verification And Documentation

**Files:**
- Modify: `PROJECT_CONTEXT.md`
- Modify: `ГДД.md`
- Modify: `AGENTS.md`
- Modify: `index.html`

- [ ] **Step 1: Run full build**

```powershell
F:/UE5/UE_5.6/Engine/Build/BatchFiles/Build.bat HeistsEditor Win64 Development -Project="F:/UE5/Projects/Heists/Heists.uproject" -WaitMutex -NoHotReload
```

Expected: succeeded.

- [ ] **Step 2: Run full Phase0 and Phase1 automation**

```powershell
F:/UE5/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe "F:/UE5/Projects/Heists/Heists.uproject" -NullRHI -Unattended -NoSplash -NoSound -DDC-ForceMemoryCache -ExecCmds="Automation RunTests Heists.Phase0; Quit" -abslog="F:/UE5/Projects/Heists/Saved/Logs/Heists_Phase0_Phase15_16.log"
F:/UE5/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe "F:/UE5/Projects/Heists/Heists.uproject" -NullRHI -Unattended -NoSplash -NoSound -DDC-ForceMemoryCache -ExecCmds="Automation RunTests Heists.Phase1; Quit" -abslog="F:/UE5/Projects/Heists/Saved/Logs/Heists_Phase1_Phase15_16.log"
```

Expected: all tests pass.

- [ ] **Step 3: Update docs**

Record:

- Phase 1.5 tactical context implemented.
- Phase 1.6 role prototype contract implemented.
- Phase 1.7 timing task hook implemented.
- Phase 1.8 mission flow contract implemented.
- User asset work remains batched for Phase 2.

- [ ] **Step 4: Commit and push**

Do not stage `Content/Heists/Characters/BP_Robber_Coordinator.uasset` unless the task explicitly requires BP visual changes.

```powershell
git add Source/Heists/Roles Source/Heists/Player Source/Heists/Character Source/Heists/Interaction Source/Heists/Game Source/Heists/UI Source/Heists/Tests PROJECT_CONTEXT.md ГДД.md AGENTS.md index.html docs/superpowers/plans/2026-05-19-phase-15-16-tactical-roles.md
git commit -m "feat: add tactical roles prototype contracts"
git push origin main
```

---

## Self-Review

- Spec coverage: tactical context, four roles, timing task hook, mission flow, network verification, and Phase 2 user asset batching are covered.
- Scope control: final AI, final animations, final assets, hub, lobby, planning, guild, and main menu implementation remain Phase 2+.
- Type consistency: role enum is `EHeistsCrewRole`, tuning struct is `FHeistsRoleTuning`, cover state is `EHeistsCoverState`, mission result is `FHeistsPrototypeMissionResult`.
