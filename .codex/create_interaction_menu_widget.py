import unreal

ASSET_DIR = "/Game/Heists/Blueprints/UI"
ASSET_NAME = "WBP_InteractionMenu"
PARENT_CLASS_PATH = "/Script/Heists.HeistsInteractionMenuWidget"


def ensure_dir(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def main() -> None:
    ensure_dir(ASSET_DIR)
    asset_path = f"{ASSET_DIR}/{ASSET_NAME}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"{asset_path} already exists")
        return

    parent_class = unreal.load_class(None, PARENT_CLASS_PATH)
    if parent_class is None:
        raise RuntimeError(f"Parent class not found: {PARENT_CLASS_PATH}")

    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset = tools.create_asset(ASSET_NAME, ASSET_DIR, None, factory)
    if asset is None:
        raise RuntimeError(f"Failed to create {asset_path}")

    unreal.EditorAssetLibrary.save_asset(asset_path)
    unreal.log(f"Created {asset_path}")


if __name__ == "__main__":
    main()
