# Phase 1 Interaction + Loot Design

## Summary

Phase 1 builds the first playable heist interaction loop on `MainMap`: players move, select contextual actions, complete simple interaction tasks, acquire shared crew access items, carry physical loot bags, and deposit them in an extraction zone.

The scope is deliberately before full stealth AI. The goal is to make doors, terminals, loot, action buttons, radial menu, progress, and replication work as a stable foundation for later stealth, roles, planning, and mission variation.

## Approved Decisions

- `MainMap` is the heist level. Hub, planning, preparation, escape, and main menu are separate future levels with separate rules/GameModes where needed.
- No full Minecraft-style or RPG-style inventory.
- Loot is mostly physical: money, gold, goods, and valuables become bags or world objects.
- Shared access items are team-level: blue/red/yellow keycards, keys, codes, clues, mission flags.
- If one player picks up a shared item, all players can benefit from it.
- Roles give advantages and new routes, but they are not hard requirements. A solo player or small team can still complete small jobs, usually slower, louder, or with more risk.
- Full custom invisible joystick is not part of Phase 1. The visible UE joystick remains for now.
- Main menu is not part of Phase 1.

## Interaction Model

The system is built around three concepts:

- `Interaction Target`: a world object such as door, terminal, loot crate, keycard pickup, loot bag, extraction zone, security panel, vault, or safe.
- `Interaction Action`: a concrete action exposed by a target, such as open, close, peek, unlock, lock, breach, hack, pickup, drop, deposit, inspect, or disable.
- `Interaction Task`: a process inside an action, such as instant action, hold progress, timing tap, fingerprint, code match, or wiring.

Objects can expose actions in two modes:

- Single-action mode: terminals and simple objects can immediately run their configured action when the player presses the action button.
- Multi-action mode: doors and complex objects can expose several actions through radial menu selection.

## Baseline Targets

Phase 1 implements or stubs the following baseline target families:

- `Terminal`: activate, hack, scan fingerprint, timing input.
- `Door`: open, close, peek/open slightly, lock, unlock, force/breach.
- `Pickup`: keycard, key, code, clue, small mission item.
- `LootContainer`: search, open, create/take loot bag.
- `LootBag`: pickup, carry, drop, deposit.
- `ExtractionZone`: deposit loot, trigger extraction progress.
- `SecurityDevice`: disable camera/alarm/panel as a server-safe stub.
- `Vault/Safe`: unlock/open flow as a primitive stub.

## Radial Menu UX

`WBP_InteractionRadialMenu` is the debug visual surface for Phase 1. Visual polish is intentionally left editable in Blueprint/UMG later.

The radial menu supports two selection styles:

- Tap mode: tap action button, radial opens, tap a sector to confirm.
- Hold mode: hold action button, radial opens, slide over a sector, release finger/mouse to confirm.

Common functions/events:

- `OpenRadialMenu(Target, Actions)`
- `CloseRadialMenu()`
- `PreviewAction(ActionId)`
- `ConfirmAction(ActionId)`
- `CancelAction()`

Action colors are part of action data:

- Green: quiet, normal, open, peek.
- Yellow: hacking, long, skill-based, timed.
- Red: loud, force, breach, dangerous.
- Blue: access, keycard, technical/system action.
- Gray: inspect, neutral, unavailable, cancel.

The radial appears near the right-side action area rather than in the screen center, so it does not cover the player, left joystick, chat, or current tasks.

## Interaction Rules

- Base actions are available to all players unless an object explicitly requires a shared access item.
- Role bonuses modify speed, noise, risk, available alternate route, or forgiveness. They do not make core objectives impossible without that role.
- One target is locked by one active player interaction at a time in Phase 1.
- Other players see that the target is busy and who is using it.
- Cancellation is allowed without penalty in Phase 1.
- Per-action progress behavior is configurable:
  - terminal/hack can preserve partial progress;
  - breach/force can reset on cancel;
  - timing task resets on failure.
- Server owns the authoritative interaction state and progress.
- Clients can request action start/cancel/confirm and receive replicated state for HUD.

## Shared Crew Items

Shared items replace a full inventory for Phase 1.

Examples:

- `BlueKeycard`
- `RedKeycard`
- `YellowKeycard`
- `Key`
- `Code`
- `Clue`
- `AccessCredential`

Shared item state should live in replicated team/game state, not on a single pawn. Pickup events should be visible to all players through debug HUD.

## Loot Rules

- Valuable heist loot is physical, not stored in a grid inventory.
- Money/gold/goods create `LootBag` actors.
- One player carries one loot bag at a time in Phase 1.
- Carrying a bag applies a simple movement speed multiplier, initially `0.75`.
- Bags can be dropped and deposited into an extraction zone.
- Weight categories and detailed bag handling are future work.

## Mini-Tasks

Phase 1 implements:

- `HoldProgress`: hold/wait until progress completes.
- `TimingTap`: tap in the correct timing window in debug form.

Phase 1 defines as stubs:

- `Fingerprint`
- `CodeMatch`
- `Wiring`

These stubs exist so terminals and safes can be authored against the final concept without delaying the first playable loop.

## Debug HUD

Phase 1 debug HUD includes:

- current interaction target;
- action button;
- radial menu;
- progress bar;
- shared crew item list;
- carried bag indicator;
- simple busy/error text.

The AI-created widgets should use stable names and events so manual visual editing can happen later without breaking gameplay logic.

## Testing

Required verification:

- C++ compile with `Build.bat HeistsEditor Win64 Development`.
- Automation tests for interaction action data, shared item state, and loot carry state where feasible.
- PIE/Standalone Listen Server + 2 Clients smoke:
  - all clients spawn in `MainMap`;
  - action button opens radial for multi-action object;
  - single-action terminal starts configured action;
  - one player locks an object while using it;
  - progress replicates;
  - shared keycard pickup is visible to every player;
  - loot bag pickup/drop/deposit replicates.

## Out Of Scope

- Final visual UI styling.
- Main menu.
- Hub, planning, preparation, escape levels.
- Full stealth AI.
- Full role ability tree.
- Full inventory.
- Dedicated Server target packaging.
