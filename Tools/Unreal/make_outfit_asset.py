"""
Create (or refresh) a CharacterOutfit data asset from imported outfit parts.

    py "C:/Dev/CountriesIRL_3D_Game/Tools/Unreal/make_outfit_asset.py" DA_Outfit_MalePeasant /Game/.../SkeletalMeshes Body Arms Legs Feet

The first listed part leads (it gets the retargeted animation); the others follow it.
"""
import sys
import unreal

name, mesh_folder, parts = sys.argv[1], sys.argv[2], sys.argv[3:]
folder = "/Game/CountriesIRL/Characters/Outfits"
retargeter = "/Game/CountriesIRL/Characters/Outfits/Retarget/RTG_Manny_To_QuaterniusOutfit"

path = folder + "/" + name
asset = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None
if not asset:
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.CharacterOutfit)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, folder, unreal.CharacterOutfit, factory)

meshes = [m for m in unreal.EditorAssetLibrary.list_assets(mesh_folder, recursive=False) if "Physics" not in m]
ordered = [next(m for m in meshes if m.split(".")[0].endswith("_" + part)) for part in parts]
asset.set_editor_property("parts", [unreal.load_asset(m) for m in ordered])
asset.set_editor_property("retargeter", unreal.load_asset(retargeter))
unreal.EditorAssetLibrary.save_loaded_asset(asset)
print("OUTFIT", path, ordered)
