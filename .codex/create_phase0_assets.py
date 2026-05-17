import unreal


def ensure_directory(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def create_blueprint(asset_path: str, parent_class_path: str):
    asset_dir, asset_name = asset_path.rsplit("/", 1)
    ensure_directory(asset_dir)

    existing = unreal.EditorAssetLibrary.load_asset(asset_path)
    if existing:
        return existing

    parent_class = unreal.load_class(None, parent_class_path)
    if not parent_class:
        raise RuntimeError(f"Parent class not found: {parent_class_path}")

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("ParentClass", parent_class)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    return asset_tools.create_asset(asset_name, asset_dir, unreal.Blueprint, factory)


def set_cdo_property(blueprint, property_name: str, value) -> None:
    generated_class = blueprint.generated_class()
    if not generated_class:
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
        generated_class = blueprint.generated_class()
    cdo = unreal.get_default_object(generated_class)
    cdo.set_editor_property(property_name, value)


def create_phase0_blueprints() -> None:
    blueprints = {
        "/Game/Heists/Blueprints/BP_HeistsGameMode": "/Script/Heists.HeistsGameMode",
        "/Game/Heists/Blueprints/BP_HeistsPlayerController": "/Script/Heists.HeistsPlayerController",
        "/Game/Heists/Blueprints/BP_HeistsGameState": "/Script/Heists.HeistsGameState",
        "/Game/Heists/Blueprints/BP_HeistsPlayerState": "/Script/Heists.HeistsPlayerState",
        "/Game/Heists/Blueprints/BP_HeistsHUD": "/Script/Heists.HeistsHUD",
        "/Game/Heists/Characters/BP_Robber_Coordinator": "/Script/Heists.HeistsRobber",
        "/Game/Heists/Characters/BP_Robber_Breaker": "/Script/Heists.HeistsRobber",
        "/Game/Heists/Characters/BP_Robber_Hacker": "/Script/Heists.HeistsRobber",
        "/Game/Heists/Characters/BP_Robber_Scout": "/Script/Heists.HeistsRobber",
        "/Game/Heists/Characters/BP_Driver": "/Script/Heists.HeistsDriver",
    }

    created = {}
    for asset_path, parent_path in blueprints.items():
        created[asset_path] = create_blueprint(asset_path, parent_path)

    role_names = {
        "/Game/Heists/Characters/BP_Robber_Coordinator": "Coordinator",
        "/Game/Heists/Characters/BP_Robber_Breaker": "Breaker",
        "/Game/Heists/Characters/BP_Robber_Hacker": "Hacker",
        "/Game/Heists/Characters/BP_Robber_Scout": "Scout",
    }

    for asset_path, role_name in role_names.items():
        set_cdo_property(created[asset_path], "RoleType", role_name)

    game_mode = created["/Game/Heists/Blueprints/BP_HeistsGameMode"]
    set_cdo_property(game_mode, "DefaultPawnClass", created["/Game/Heists/Characters/BP_Robber_Coordinator"].generated_class())
    set_cdo_property(game_mode, "PlayerControllerClass", created["/Game/Heists/Blueprints/BP_HeistsPlayerController"].generated_class())
    set_cdo_property(game_mode, "GameStateClass", created["/Game/Heists/Blueprints/BP_HeistsGameState"].generated_class())
    set_cdo_property(game_mode, "PlayerStateClass", created["/Game/Heists/Blueprints/BP_HeistsPlayerState"].generated_class())
    set_cdo_property(game_mode, "HUDClass", created["/Game/Heists/Blueprints/BP_HeistsHUD"].generated_class())

    for blueprint in created.values():
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
        unreal.EditorAssetLibrary.save_loaded_asset(blueprint)


def spawn_static_mesh_actor(mesh_path: str, label: str, location, scale) -> None:
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    actor = actor_subsystem.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*location))
    actor.set_actor_label(label)
    actor.set_actor_scale3d(unreal.Vector(*scale))
    actor.static_mesh_component.set_static_mesh(mesh)


def create_phase0_map() -> None:
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    level_subsystem.load_level("/Game/Maps/MainMap")

    for actor in list(actor_subsystem.get_all_level_actors()):
        label = actor.get_actor_label()
        if label.startswith("Phase0_"):
            actor_subsystem.destroy_actor(actor)

    cube_path = "/Engine/BasicShapes/Cube.Cube"

    spawn_static_mesh_actor(cube_path, "Phase0_Floor", (0.0, 0.0, -55.0), (26.0, 18.0, 1.0))
    spawn_static_mesh_actor(cube_path, "Phase0_Bank_BackWall", (0.0, 900.0, 150.0), (26.0, 0.5, 4.0))
    spawn_static_mesh_actor(cube_path, "Phase0_Bank_FrontWall_Left", (-750.0, -900.0, 150.0), (11.0, 0.5, 4.0))
    spawn_static_mesh_actor(cube_path, "Phase0_Bank_FrontWall_Right", (750.0, -900.0, 150.0), (11.0, 0.5, 4.0))
    spawn_static_mesh_actor(cube_path, "Phase0_Bank_LeftWall", (-1300.0, 0.0, 150.0), (0.5, 18.0, 4.0))
    spawn_static_mesh_actor(cube_path, "Phase0_Bank_RightWall", (1300.0, 0.0, 150.0), (0.5, 18.0, 4.0))
    spawn_static_mesh_actor(cube_path, "Phase0_Vault_Blockout", (800.0, 450.0, 100.0), (6.0, 5.0, 2.5))
    spawn_static_mesh_actor(cube_path, "Phase0_Teller_Counter", (-350.0, 250.0, 65.0), (8.0, 0.8, 1.3))

    nav_volume = actor_subsystem.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(0.0, 0.0, 120.0))
    nav_volume.set_actor_label("Phase0_NavMeshBounds")
    nav_volume.set_actor_scale3d(unreal.Vector(28.0, 20.0, 4.0))

    player_start_class = unreal.PlayerStart
    for index, location in enumerate([(-300.0, -500.0, 120.0), (-100.0, -500.0, 120.0), (100.0, -500.0, 120.0), (300.0, -500.0, 120.0)]):
        player_start = actor_subsystem.spawn_actor_from_class(player_start_class, unreal.Vector(*location))
        player_start.set_actor_label(f"Phase0_PlayerStart_{index + 1}")

    level_subsystem.save_current_level()


create_phase0_blueprints()
create_phase0_map()
unreal.EditorAssetLibrary.save_directory("/Game/Heists", only_if_is_dirty=False, recursive=True)
unreal.log("Phase 0 Heists blueprints and primitive map are ready.")
