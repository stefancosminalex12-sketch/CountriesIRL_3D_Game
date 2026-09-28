"""
Import the menu art (Art/AI/Menu/*.png) as UI textures. Run inside the editor:
    py "C:/Dev/CountriesIRL_3D_Game/Tools/Unreal/import_ui_textures.py"
Textures are named T_<file name> in /Game/CountriesIRL/UI/Textures, set up for UI:
no mipmaps, never streamed (stays sharp), UI texture group and compression.
"""
import glob
import os
import unreal

SOURCE = "C:/Dev/CountriesIRL_3D_Game/Art/AI/Menu"
DESTINATION = "/Game/CountriesIRL/UI/Textures"

tasks = []
for path in sorted(glob.glob(os.path.join(SOURCE, "*.png"))):
    task = unreal.AssetImportTask()
    task.filename = path.replace("\\", "/")
    task.destination_path = DESTINATION
    task.destination_name = "T_" + os.path.splitext(os.path.basename(path))[0]
    task.automated = True
    task.replace_existing = True
    task.save = False
    tasks.append(task)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

for task in tasks:
    for object_path in task.imported_object_paths:
        texture = unreal.load_asset(object_path)
        if not isinstance(texture, unreal.Texture2D):
            continue
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
        texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        texture.set_editor_property("never_stream", True)
        # The panel texture repeats across the panel
        tiles = "panel_texture" in object_path
        address = unreal.TextureAddress.TA_WRAP if tiles else unreal.TextureAddress.TA_CLAMP
        texture.set_editor_property("address_x", address)
        texture.set_editor_property("address_y", address)
        unreal.EditorAssetLibrary.save_loaded_asset(texture)
        print("IMPORT_RESULT", task.filename, "->", object_path)
