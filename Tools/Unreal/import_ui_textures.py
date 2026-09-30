"""
Import the menu art (Art/AI/Menu/*.png, plus our own Art/UI/*.png such as gradients) as UI textures. Run inside the editor:
    py "C:/Dev/CrownsAndCommoners/Tools/Unreal/import_ui_textures.py" [part of a file name, e.g. gradient]
Textures are named T_<file name> in /Game/CrownsAndCommoners/UI/Textures (icons in /Game/CrownsAndCommoners/UI/Icons), set up for UI:
no mipmaps, never streamed (stays sharp), UI texture group and compression.
"""
import glob
import sys
import os
import unreal

# Source folder -> content folder. Item icons come from Tools/prepare_icons.py
SOURCES = {
    unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()) + "Art/AI/Menu": "/Game/CrownsAndCommoners/UI/Textures",
    unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()) + "Art/UI": "/Game/CrownsAndCommoners/UI/Textures",
    unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()) + "Art/UI/Icons": "/Game/CrownsAndCommoners/UI/Icons",
    unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()) + "Art/UI/Markers": "/Game/CrownsAndCommoners/UI/Markers",
}

tasks = []
only = sys.argv[1] if len(sys.argv) > 1 else ""   # optional: import just the files whose name contains this
paths = [(p, dest) for source, dest in SOURCES.items() for p in sorted(glob.glob(os.path.join(source, "*.png"))) if only in os.path.basename(p)]
for path, destination in paths:
    task = unreal.AssetImportTask()
    task.filename = path.replace("\\", "/")
    task.destination_path = destination
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
        # New assets aren't always marked dirty after import: save regardless
        unreal.EditorAssetLibrary.save_asset(object_path.split(".")[0], False)
        print("IMPORT_RESULT", task.filename, "->", object_path)
