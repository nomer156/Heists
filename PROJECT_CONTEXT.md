=== HEISTS — PROJECT CONTEXT (Память агента) ===
Последнее обновление: 2026-05-18

--- ТЕКУЩИЙ СТАТУС ---
Phase: 1.5+1.6 — Tactical Context + Role Prototype contracts implemented; Phase 1 interaction/loot работает, portrait-first input/HUD есть, роли/cover/auto-context/timing/mission result добавлены как C++ прототипные контракты.
UE версия: 5.6
Движок: F:\UE5\UE_5.6\
Проект: F:\UE5\Projects\Heists\
Git: локально инициализируется в этом этапе; remote: https://github.com/nomer156/Heists

--- ПРИНЯТЫЕ АРХИТЕКТУРНЫЕ РЕШЕНИЯ ---

[МУЛЬТИПЛЕЕР]
- Dedicated Server в конце разработки.
- Тестирование: Play as Listen Server + Client в редакторе UE.
- Вся gameplay-логика пишется с учётом репликации и server authority.
- Client input допускается только как prediction/request; итоговое состояние важно держать на сервере.

[GAS OWNERSHIP — ПРИНЯТО 2026-05-17]
- AHeistsPlayerState реализует IAbilitySystemInterface.
- AHeistsPlayerState владеет AbilitySystemComponent и UHeistsAttributeSet.
- AHeistsCharacterBase не создаёт ASC subobject; он берёт ASC из PlayerState и становится avatar через InitAbilityActorInfo(PlayerState, Character).
- Причина: Lyra-style, безопаснее для МП, respawn, possession и смены pawn.

[ИЕРАРХИЯ КЛАССОВ — ВЫБРАНА ОПЦИЯ 1]
Решение: C++ base → C++ role → Blueprint (visual/defaults)

  AHeistsPlayerState (C++)            — GAS owner, роли, очки, ready/alive state
  AHeistsCharacterBase (C++)          — avatar, движение, камера, базовое состояние
    ├── AHeistsRobber (C++)           — взаимодействие, лут, роль грабителя
    │     ├── BP_Robber_Coordinator   — visual/defaults
    │     ├── BP_Robber_Breaker       — visual/defaults
    │     ├── BP_Robber_Hacker        — visual/defaults
    │     └── BP_Robber_Scout         — visual/defaults
    └── AHeistsDriver (C++)           — водитель, эвакуация, транспортные stubs
          └── BP_Driver               — visual/defaults

GameMode: AHeistsGameMode (C++) → BP_HeistsGameMode
PlayerController: AHeistsPlayerController (C++) → BP_HeistsPlayerController
GameState: AHeistsGameState (C++) → BP_HeistsGameState
PlayerState: AHeistsPlayerState (C++) → BP_HeistsPlayerState
HUD: AHeistsHUD (C++) → BP_HeistsHUD

[УПРАВЛЕНИЕ — ОБНОВЛЕНО 2026-05-18]
- Основной режим: mobile portrait.
- Portrait layout: верх экрана — цели/статус/миссия; середина — gameplay view и свободная camera-drag зона; низ — видимый на прототипе virtual joystick, interact/drop/action buttons, быстрые команды.
- Landscape layout остаётся fallback при повороте телефона: левая половина — joystick/chat/tasks, правая — camera drag/actions/progress.
- UI-зоны в активном layout должны consume input: тап по задачам/чату/кнопкам не двигает персонажа и не вращает камеру.
- C++ AHeistsPlayerController задаёт `IMC_Default` + `IA_Move`; дочерние BP не настраивают movement отдельно.
- Editor/Standalone fallback: WASD работает через `IA_Move`, мышь симулирует touch, virtual joystick включён, окна 720x1280 для portrait smoke.
- Camera drag реализован в `AHeistsPlayerController`: в portrait работает по свободной центральной зоне, в landscape — по свободной правой стороне; зарезервированные UI-зоны не вращают камеру.
- Click-to-move остаётся как дополнительный dev fallback.
- AHeistsPlayerController содержит IA_Move, IA_ClickMove, adaptive mobile block zones, layout detection и Server_TriggerAbilitySlot.

[АРТ-СТИЛЬ]
- Tacticool-style: мультяшный, но реалистичный, не low-poly.
- Контурная обводка персонажей и интерактивных объектов.
- Mobile-friendly visual target, 60-120 FPS.
- Сейчас: primitive bank blockout и базовая UE геометрия.
- `BP_Robber_Coordinator.uasset` считается обычным visual BP ребёнком; временный mesh/animation пользователя допустим и не блокирует C++ работу.
- Fab assets пока не импортируем в Phase 1. На Phase 2 готовим единый asset pass; low poly сразу отбрасываем.

[РЕЖИМЫ ИГРЫ]
- Основной: 4 игрока.
- Solo запуск подготовок/ограблений поддерживается архитектурно.
- Future: mini prep 1-2 игрока, большие ивенты 4+, PvP 4v4.

[PHASE 1 — INTERACTION + LOOT, УТВЕРЖДЕНО 2026-05-17]
- `MainMap` отвечает именно за ограбление.
- Подготовка, планирование, хаб, побег и главное меню — отдельные будущие уровни с отдельными правилами/GameMode при необходимости.
- Interaction model: `Target -> Action -> Task`.
- Single-action объекты, например терминал, сразу запускают заранее выбранное действие.
- Multi-action объекты, например дверь, открывают radial menu с выбором действия.
- Radial UX: `tap -> tap sector` и `hold -> slide -> release`; PC/editor повторяет это мышью.
- Цвета действий: Green quiet/open/peek, Yellow hack/long/skill, Red force/breach/noisy, Blue access/tech, Gray inspect/disabled/cancel.
- Роли дают преимущества и альтернативные пути, но не являются обязательными условиями. Solo/малые задания должны быть возможны, но могут быть дольше, шумнее или рискованнее.
- Полноценный grid-инвентарь не нужен.
- Shared crew items: keycards, keys, codes, clues, mission flags. Если один игрок подобрал предмет, доступ получает вся команда.
- Добыча физическая: деньги/золото/товар в сумках; один игрок несёт одну сумку; базовый speed multiplier с сумкой 0.75.
- Один объект в Phase 1 использует один игрок одновременно; остальные видят busy state.
- Отмена без штрафа в Phase 1; последствия добавим позже.
- Mini-tasks Phase 1: `HoldProgress`, `TimingTap`; `Fingerprint`, `CodeMatch`, `Wiring` как stubs.
- Debug HUD делает Codex: action button, button-menu/radial menu, progress, shared items, carried bag. Пользователь позже вручную редактирует визуал.
- Интерактивная цель локально подсвечивается зелёным через `SetLocallyFocused`; это client-only hint, не gameplay state.
- `WBP_InteractionMenu` находится в `/Content/UI` как BP-наследник `UHeistsInteractionMenuWidget`; сейчас показывает действия кнопками, сохраняя `E` + `1-6` debug fallback.
- `WBP_MobileHUD` находится в `/Content/UI` как BP-наследник `UHeistsMobileHUDWidget`. Если Blueprint пустой, native-класс сам строит простой debug layout с кнопками `Interact`/`Drop`. Стабильные designer names: `Panel_Objectives`, `Panel_QuickCommands`, `Panel_PortraitRoot`, `Panel_LandscapeRoot`, `Button_Interact`, `Button_DropBag`, `ActionList`.
- Ручной визуальный polish UI, персонажей, объектов, ассетов и анимаций сдвигается пакетом к старту Phase 2, чтобы пользователь делал настройку не по чуть-чуть.

[PHASE 1.5 + 1.6 — УТВЕРЖДЕНО 2026-05-19]
- Подход: крупные связанные batch-изменения вместо одной мелкой механики за раз.
- Design spec: `docs/superpowers/specs/2026-05-19-phase-15-16-tactical-roles-design.md`.
- Phase 1.5 результат: usable cover/wall-stick/peek foundation и auto-context hints для дверей/сейфов/контейнеров.
- Phase 1.6 результат: 4 роли имеют gameplay-различия без hard-lock solo completion.
- Phase 1.7 результат: `TimingTap` playable debug, hack/fingerprint/code/wiring остаются стабильными stubs.
- Phase 1.8 результат: один `MainMap` heist flow со start/objective/loot/extraction/result.
- Phase 1.9 результат: Listen Server + 2-4 clients smoke без поломки replicated state.
- Main menu позже делается отдельной картой с отдельным pawn/camera setup, не статичной картинкой.

[ТЕСТИРОВАНИЕ]
- C++ build command:
  F:/UE5/UE_5.6/Engine/Build/BatchFiles/Build.bat HeistsEditor Win64 Development -Project="F:/UE5/Projects/Heists/Heists.uproject" -WaitMutex -NoHotReload
- Automation:
  UnrealEditor-Cmd.exe Heists.uproject -NullRHI -Unattended -NoSplash -NoSound -DDC-ForceMemoryCache -ExecCmds="Automation RunTests Heists.Phase0; Quit"
- Без финальных mesh/animation/sound тестируем capsule, movement, PlayerState, ASC, AttributeSet, possession/init path и replication-ready API.

--- ЧТО СДЕЛАНО ---
[x] UE5 проект Heists.uproject (UE 5.6)
[x] C++ модуль Heists
[x] GameplayAbilities, EnhancedInput, CommonUI, Niagara включены
[x] UnrealMCP plugin подключён
[x] AHeistsCharacterBase, AHeistsRobber, AHeistsDriver
[x] AHeistsGameMode, AHeistsGameState, AHeistsPlayerState, AHeistsPlayerController, AHeistsHUD
[x] GAS перенесён на PlayerState
[x] Mobile adaptive input contract добавлен в PlayerController
[x] BP_Heists* и BP_Robber*/BP_Driver созданы
[x] MainMap обновлена primitive bank blockout + NavMeshBounds + PlayerStarts
[x] Project defaults переключены с TopDown template на BP_HeistsGameMode
[x] MainMap закреплена на BP_HeistsGameMode как world override
[x] Automation tests Heists.Phase0 добавлены
[x] DDC startup crash исправлен: project-level InstalledDerivedDataBackendGraph использует writable DerivedDataCache и не ждёт ZenLocal
[x] Mobile input defaults исправлены: Engine LeftVirtualJoystickOnly, mouse-as-touch, always-show touch interface
[x] BP_HeistsPlayerController получает IMC_Default + IA_Move из C++/BP defaults
[x] Standalone/PIE основной smoke window portrait 720x1280; landscape остаётся fallback для широкого viewport
[x] Удалён template-контент TopDown и Variant_TwinStick
[x] Build.bat HeistsEditor Win64 Development — успешно
[x] Automation Heists.Phase0 — 3/3 success
[x] Phase 1 design spec утверждён и сохранён
[x] Phase 1 implementation plan сохранён
[x] `FHeistsInteractionAction` и enums `ActionId/TaskType/Color/ProgressBehavior`
[x] `AHeistsGameState` реплицирует shared crew items через `FGameplayTagContainer`
[x] `IHeistsInteractable` + `UHeistsInteractionComponent`
[x] `AHeistsInteractableActorBase`, `AHeistsDoorActor`, `AHeistsTerminalActor`
[x] `AHeistsPickupActor` для shared keycards/keys/codes/clues
[x] `AHeistsLootBag`, `AHeistsLootContainer`, `AHeistsExtractionZone`
[x] `AHeistsRobber` хранит одну carried loot bag и применяет speed multiplier 0.75
[x] `AHeistsPlayerController` умеет открыть radial debug path, подтвердить action и отменить
[x] `AHeistsHUD` рисует debug target/actions/progress/shared items/carried bag
[x] `AHeistsGameMode` runtime-спавнит Phase 1 prototype actors на MainMap
[x] Right-side/central camera drag, local interaction highlight, `WBP_InteractionMenu` button menu и `DropCarriedLoot`/`G`
[x] `WBP_InteractionMenu` перенесён в `/Content/UI`; создан `/Content/UI/WBP_MobileHUD`
[x] `UHeistsMobileHUDWidget` добавлен как native adaptive HUD contract
[x] `EHeistsCrewRole` + `FHeistsRoleTuning`; `AHeistsPlayerState` реплицирует `CrewRole`
[x] `UHeistsCoverComponent` расширен до `None/InCover/Peeking` с server-safe enter/exit/toggle peek
[x] `UHeistsInteractionComponent` выбирает best contextual action и считает role-adjusted duration
[x] `FHeistsTimingTapTask` добавлен как debug timing hook
[x] `AHeistsGameState` реплицирует `FHeistsPrototypeMissionResult` и start/complete/fail prototype mission flow
[x] `AHeistsHUD` показывает role, cover state и mission result debug lines
[x] Automation `Heists.Phase1` — 13/13 success

--- СЛЕДУЮЩИЕ ШАГИ ---
1. Ручной Standalone/PIE Listen Server + 2 Clients smoke на MainMap в portrait 720x1280.
2. Проверить rotation fallback: широкий viewport/landscape должен вернуть старую схему правой camera-drag зоны.
3. Проверить `WBP_MobileHUD`: `Button_Interact` вызывает `OpenInteractionRadial`, `Button_DropBag` вызывает `DropCarriedLoot`.
4. Проверить debug HUD: role, cover state, mission result, loot/shared items.
5. Подключить role defaults к BP_Robber_* визуальным детям без gameplay logic в Blueprint.
6. Следующий крупный кодовый batch: Phase 1.8 runtime prototype mission objective chain на MainMap + 4-player smoke helpers.
7. На старте Phase 2 выдать пользователю единый список ручных задач: меши/анимации персонажей, visual polish UI, placeholder ассеты объектов, материалы интерактива и читаемость карты.

--- ВАЖНЫЕ ЗАМЕТКИ ---
- DDC фикс находится в `Config/DefaultEngine.ini`: Local cache пишет в `%GAMEDIR%DerivedDataCache`, ZenLocal исключён из project-level hierarchy.
- UnrealMCP порт 55557 может быть занят уже запущенным сервером; это не блокирует C++ build/automation.
- Touch joystick сейчас использует engine asset `/Engine/MobileResources/HUD/LeftVirtualJoystickOnly`. Позже заменим на собственный невидимый UMG joystick/chat/tasks layout.
- `Content/Input/Touch/UI_TouchSimple` и `UI_Thumbstick` — WidgetBlueprint assets, не `UTouchInterface`; для стандартного UE virtual joystick нельзя указывать их в `DefaultTouchInterface`.
- Python/editor map-spawn через `EditorActorSubsystem` в текущей среде падает внутри UE ActorFactory с `EXCEPTION_INT_DIVIDE_BY_ZERO`; поэтому Phase 1 prototype actors временно спавнятся runtime в `AHeistsGameMode` только на `MainMap`.
- `ГДД.md`, `index.html`, `AGENTS.md`, `PROJECT_CONTEXT.md` — актуальные living docs.
