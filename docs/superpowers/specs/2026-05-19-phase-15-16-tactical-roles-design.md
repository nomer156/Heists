# Phase 1.5 + 1.6 Tactical Context And Role Prototype Design

## Summary

This phase finishes the next large prototype layer before Phase 2. The goal is not final stealth, not final animation, and not final assets. The goal is a playable `MainMap` prototype where four roles have clear gameplay identity, contextual world actions are predictable, and tactical movement starts to feel like a heist game.

Implementation should be grouped into large connected batches. Avoid adding one isolated mechanic at a time if the same controller, robber, interaction, HUD, and tests would be touched repeatedly.

## Accepted Working Rules

- `BP_Robber_Coordinator.uasset` is a normal visual child Blueprint. The temporary mesh/animations are allowed and should not block C++ work.
- Blueprint visual changes are acceptable in role/character BP children when the change is only mesh, animation, sound, camera, or visual defaults.
- Gameplay, replication, roles, cover, interaction, tasks, and mission state stay in C++.
- Phase 1 remains asset-light: no final art, no final animation, no final sound.
- User asset work is batched for Phase 2 unless a placeholder is required to test gameplay.

## Asset Direction For Later

Do not import these into the Phase 1 prototype yet. Use this as the first shortlist when Phase 2 asset pass starts.

Preferred families from the current Fab screenshots:

- Characters: `Quantum Modular Character Free Sample`, `Stylized Modular Character`, `Adventure Character`, `Scanned 3D People Pack`, `City Sample Crowds`.
- Animation: `Game Animation Sample`, `Free Sample Animation Set`, `Free Animation Library`, `Close Combat Animset`.
- Heist/city locations: `Supermarket`, `Big Office`, `Modular Building Set`, `Assetsville Town`, `City Sample Buildings`, `Underground Subway`, `Factory Environment Collection`.
- Vehicles/city dressing: `City Sample Vehicles`, `Truck and Trailers`, `Construction Vehicles Pack`.
- VFX: `Realistic Starter VFX Pack`, `M5VFX`, `Niagara Mega VFX Pack`, `Hack And Slash FX` for selective sparks/impact/debug use.
- UI reference only: `Generic Radial Menus v2.0`.

Reject for the main visual direction:

- Clearly low-poly packs.
- Fantasy/medieval/magic packs as primary content.
- Full gameplay kits that would replace our architecture.
- Heavy realistic megascans environments unless used sparingly as dressing.

## Menu Direction

The main menu will not be a static background. It will be a separate map with its own pawn/camera setup. Later this allows a living lobby/menu scene where characters, vehicles, planning board, guild state, and decorations can change.

Main menu, lobby, hub, planning, preparation missions, guild systems, and escape flow are Phase 2+ work. Phase 1 only keeps the architecture compatible with them.

## Phase 1.X Results

- Phase 1.1 Interaction Core: players see a target, choose an action, and execute through a server-safe path.
- Phase 1.2 Loot Core: loot bags can be picked up, carried, dropped, and deposited.
- Phase 1.3 Shared Crew Items: cards, keys, codes, clues, and mission flags are shared by the crew.
- Phase 1.4 Mobile HUD/Input: portrait is primary, landscape is fallback, touch/mouse/WASD work for testing.
- Phase 1.5 Tactical Context: cover, wall-stick, peek, and auto-context hints exist as a prototype.
- Phase 1.6 Role Prototype: four roles have gameplay differences without hard-locking solo completion.
- Phase 1.7 Mini Tasks: hold, timing tap, and hack/fingerprint/code/wiring stubs exist behind the same task API.
- Phase 1.8 Prototype Mission: one `MainMap` heist has start, objective, loot, extraction, and result flow.
- Phase 1.9 Network Smoke: Listen Server + 2-4 clients can complete the prototype without broken replicated state.

## Phase 1.5 Tactical Context Scope

Results:

- `UHeistsCoverComponent` becomes usable, not just a replicated data holder.
- Nearby valid cover can be detected and exposed to HUD/debug output.
- Player can enter/leave cover through a server-safe request.
- Cover state affects movement/camera/interaction enough to test the idea without animations.
- Peek is represented as a gameplay state/action, even if visual lean animation is not final.
- Doors, terminals, safes, containers, and loot expose a "best contextual action" for mobile UX.
- Auto-context means the UI proposes the best action; it does not mean the client commits gameplay state directly.

Non-goals:

- Final cover animation.
- Final stealth AI.
- Final wall alignment polish.
- Final camera collision or cinematic movement.

## Phase 1.6 Four Role Prototype Scope

Roles:

- Coordinator: safer shared-item/team utility path, better command/debug information, future planning synergy.
- Hacker: faster/safer terminal and electronic interactions, stronger hack mini-task results.
- Breaker: faster/lower-failure force actions, breach/door/container advantage, louder by default.
- Scout: better detection, peek/range/target awareness, faster quiet context interactions.

Rules:

- Roles give advantages and alternate paths.
- No role is mandatory for the only completion route.
- Solo completion remains possible, but slower, noisier, riskier, or with worse rewards.
- Role state should live in replicated gameplay state and be readable by HUD.
- Role bonuses should be data-driven enough to tune later, but not over-abstracted before the prototype proves itself.

## Phase 1.7 Mini Task Scope

Results:

- `HoldProgress` remains the stable base task.
- `TimingTap` becomes playable in debug form.
- `Hack`, `Fingerprint`, `CodeMatch`, and `Wiring` stay behind stable C++ task hooks as stubs.
- Actions can select task type and role modifiers can affect task duration, failure tolerance, or progress preservation.

## Phase 1.8 Prototype Mission Scope

Results:

- `MainMap` can run a simple heist from start to extraction.
- Four role pawns can join the same session.
- There is one objective chain: enter, interact with at least one access object, collect loot, extract.
- Mission result is displayed in debug HUD/log: success/fail, loot delivered, shared items acquired.
- Runtime-spawned prototype actors remain acceptable until editor placement is stable.

## Phase 1.9 Network Smoke Scope

Results:

- Listen Server + 2 clients is the minimum recurring smoke.
- Listen Server + 4 players is the target smoke before Phase 2.
- Interaction locks, role state, shared items, carried bags, dropped bags, and extraction result replicate correctly.
- The prototype can be tested without final meshes, animation, sound, or imported assets.

## Implementation Shape

The next implementation plan should group edits by systems:

- Role/mission state batch: role enum, role config, role assignment, debug HUD display, tests.
- Tactical context batch: cover enter/exit/peek, best contextual action, debug HUD display, tests.
- Mini-task batch: timing tap and task hooks, role modifiers, tests.
- Mission flow batch: start/objective/extraction/result, network smoke hooks, tests.

This keeps repeated edits to controller, robber, interaction component, HUD, tests, and docs concentrated in fewer passes.

## Validation

- Build: `HeistsEditor Win64 Development`.
- Automation: full `Heists.Phase0` and `Heists.Phase1`.
- Add/extend tests for Phase 1.5/1.6 contracts before implementation.
- Manual smoke: Standalone/PIE Listen Server + clients on `MainMap`.

## Phase 2 Handoff

At the start of Phase 2, provide one consolidated user checklist for:

- Character meshes and animation choices.
- UI visual polish for `WBP_MobileHUD` and `WBP_InteractionMenu`.
- Prototype object art for doors, terminals, safes, keycards, bags, extraction zone.
- Map readability and blockout upgrade.
- First asset shortlist import order.
