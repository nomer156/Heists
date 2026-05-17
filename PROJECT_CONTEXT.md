=== HEISTS — PROJECT CONTEXT (Память агента) ===
Последнее обновление: 2026-05-17

--- ТЕКУЩИЙ СТАТУС ---
Phase: 0 — Основа проекта (БАЗОВАЯ АРХИТЕКТУРА СОЗДАНА; input/defaults cleanup выполнен; следующий шаг — утвердить Phase 1 interaction + loot vertical slice)
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

[УПРАВЛЕНИЕ — ПРИНЯТО 2026-05-17]
- Основной режим: mobile landscape.
- Левая половина экрана: невидимый virtual joystick, чат, текущие задания.
- Левые UI-зоны чата/заданий должны consume input и не двигать персонажа.
- Правая половина экрана: interact/action/ability-кнопки, иконки, прогресс.
- C++ AHeistsPlayerController задаёт `IMC_Default` + `IA_Move`; дочерние BP не настраивают movement отдельно.
- Editor/Standalone fallback: WASD работает через `IA_Move`, мышь симулирует touch, virtual joystick включён, окна 1280x720.
- Click-to-move остаётся как дополнительный dev fallback.
- AHeistsPlayerController содержит IA_Move, IA_ClickMove, mobile block zones и Server_TriggerAbilitySlot.

[АРТ-СТИЛЬ]
- Tacticool-style: мультяшный, но реалистичный, не low-poly.
- Контурная обводка персонажей и интерактивных объектов.
- Mobile-friendly visual target, 60-120 FPS.
- Сейчас: primitive bank blockout и базовая UE геометрия.

[РЕЖИМЫ ИГРЫ]
- Основной: 4 игрока.
- Solo запуск подготовок/ограблений поддерживается архитектурно.
- Future: mini prep 1-2 игрока, большие ивенты 4+, PvP 4v4.

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
[x] Mobile landscape input contract добавлен в PlayerController
[x] BP_Heists* и BP_Robber*/BP_Driver созданы
[x] MainMap обновлена primitive bank blockout + NavMeshBounds + PlayerStarts
[x] Project defaults переключены с TopDown template на BP_HeistsGameMode
[x] MainMap закреплена на BP_HeistsGameMode как world override
[x] Automation tests Heists.Phase0 добавлены
[x] DDC startup crash исправлен: project-level InstalledDerivedDataBackendGraph использует writable DerivedDataCache и не ждёт ZenLocal
[x] Mobile input defaults исправлены: Engine LeftVirtualJoystickOnly, mouse-as-touch, always-show touch interface
[x] BP_HeistsPlayerController получает IMC_Default + IA_Move из C++/BP defaults
[x] Standalone/PIE окна настроены landscape 1280x720
[x] Удалён template-контент TopDown и Variant_TwinStick
[x] Build.bat HeistsEditor Win64 Development — успешно
[x] Automation Heists.Phase0 — 2/2 success

--- СЛЕДУЮЩИЕ ШАГИ ---
1. Пользователь утверждает Phase 1 scope: interaction contract + loot vertical slice.
2. Реализовать `IHeistsInteractable`, `UHeistsInteractionComponent`, replicated progress и action-button path.
3. Добавить тестовые объекты: дверь, терминал, loot crate, extraction zone.
4. Добавить loot bag pickup/carry/drop/extract flow.
5. Прогнать Listen Server + 2 Clients smoke после реализации.

--- ВАЖНЫЕ ЗАМЕТКИ ---
- DDC фикс находится в `Config/DefaultEngine.ini`: Local cache пишет в `%GAMEDIR%DerivedDataCache`, ZenLocal исключён из project-level hierarchy.
- UnrealMCP порт 55557 может быть занят уже запущенным сервером; это не блокирует C++ build/automation.
- Touch joystick сейчас использует engine asset `/Engine/MobileResources/HUD/LeftVirtualJoystickOnly`. Позже заменим на собственный невидимый UMG joystick/chat/tasks layout.
- `Content/Input/Touch/UI_TouchSimple` и `UI_Thumbstick` — WidgetBlueprint assets, не `UTouchInterface`; для стандартного UE virtual joystick нельзя указывать их в `DefaultTouchInterface`.
- `ГДД.md`, `index.html`, `AGENTS.md`, `PROJECT_CONTEXT.md` — актуальные living docs.
