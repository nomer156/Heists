import unreal

ROOT_UI_DIR = "/Game/UI"
OLD_INTERACTION_MENU = "/Game/Heists/Blueprints/UI/WBP_InteractionMenu"
NEW_INTERACTION_MENU = f"{ROOT_UI_DIR}/WBP_InteractionMenu"
MOBILE_HUD_NAME = "WBP_MobileHUD"
MOBILE_HUD_PATH = f"{ROOT_UI_DIR}/{MOBILE_HUD_NAME}"


def ensure_dir(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def create_widget_blueprint(asset_name: str, asset_dir: str, parent_class_path: str) -> None:
    asset_path = f"{asset_dir}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"{asset_path} already exists")
        return

    parent_class = unreal.load_class(None, parent_class_path)
    if parent_class is None:
        raise RuntimeError(f"Parent class not found: {parent_class_path}")

    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset = asset_tools.create_asset(asset_name, asset_dir, None, factory)
    if asset is None:
        raise RuntimeError(f"Failed to create {asset_path}")

    unreal.EditorAssetLibrary.save_asset(asset_path)
    unreal.log(f"Created {asset_path}")


def main() -> None:
    ensure_dir(ROOT_UI_DIR)

    if unreal.EditorAssetLibrary.does_asset_exist(OLD_INTERACTION_MENU):
        if unreal.EditorAssetLibrary.does_asset_exist(NEW_INTERACTION_MENU):
            unreal.EditorAssetLibrary.delete_asset(OLD_INTERACTION_MENU)
            unreal.log(f"Deleted duplicate {OLD_INTERACTION_MENU}")
        else:
            if not unreal.EditorAssetLibrary.rename_asset(OLD_INTERACTION_MENU, NEW_INTERACTION_MENU):
                raise RuntimeError(f"Failed to move {OLD_INTERACTION_MENU} to {NEW_INTERACTION_MENU}")
            unreal.log(f"Moved {OLD_INTERACTION_MENU} to {NEW_INTERACTION_MENU}")

    create_widget_blueprint(
        MOBILE_HUD_NAME,
        ROOT_UI_DIR,
        "/Script/Heists.HeistsMobileHUDWidget")

    unreal.EditorAssetLibrary.save_directory(ROOT_UI_DIR)


if __name__ == "__main__":
    main()
