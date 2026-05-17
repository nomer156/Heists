import unreal


def load(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def load_class(path):
    cls = unreal.EditorAssetLibrary.load_blueprint_class(path)
    if not cls:
        raise RuntimeError(f"Missing blueprint class: {path}")
    return cls


def set_property(obj, names, value):
    last_error = None
    for name in names:
        try:
            obj.set_editor_property(name, value)
            return True
        except Exception as exc:
            last_error = exc
    raise RuntimeError(f"Unable to set {names} on {obj}: {last_error}")


mapping_context = load("/Game/Input/IMC_Default")
move_action = load("/Game/Input/Actions/IA_Move")

pc_bp_class = load_class("/Game/Heists/Blueprints/BP_HeistsPlayerController")
pc_cdo = unreal.get_default_object(pc_bp_class)
set_property(pc_cdo, ["DefaultMappingContext", "default_mapping_context"], mapping_context)
set_property(pc_cdo, ["IA_Move", "ia_move"], move_action)
set_property(pc_cdo, ["bUseMobileStickMovement", "use_mobile_stick_movement"], True)

game_mode_bp_class = load_class("/Game/Heists/Blueprints/BP_HeistsGameMode")
game_mode_cdo = unreal.get_default_object(game_mode_bp_class)
set_property(game_mode_cdo, ["PlayerControllerClass", "player_controller_class"], pc_bp_class)
set_property(game_mode_cdo, ["DefaultPawnClass", "default_pawn_class"], load_class("/Game/Heists/Characters/BP_Robber_Coordinator"))
set_property(game_mode_cdo, ["GameStateClass", "game_state_class"], load_class("/Game/Heists/Blueprints/BP_HeistsGameState"))
set_property(game_mode_cdo, ["PlayerStateClass", "player_state_class"], load_class("/Game/Heists/Blueprints/BP_HeistsPlayerState"))
set_property(game_mode_cdo, ["HUDClass", "hud_class"], load_class("/Game/Heists/Blueprints/BP_HeistsHUD"))

unreal.EditorAssetLibrary.save_loaded_asset(load("/Game/Heists/Blueprints/BP_HeistsPlayerController"))
unreal.EditorAssetLibrary.save_loaded_asset(load("/Game/Heists/Blueprints/BP_HeistsGameMode"))

unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/MainMap")
world = unreal.EditorLevelLibrary.get_editor_world()
world_settings = world.get_world_settings()
set_property(world_settings, ["DefaultGameMode", "default_game_mode"], game_mode_bp_class)
unreal.EditorLoadingAndSavingUtils.save_current_level()

unreal.log("Phase 0 input/default classes configured.")
