"""
Import the game's world map (made by Tools/world/export_game_map.py) and write its calibration into the map data asset.

In the editor:   py "C:/Dev/CountriesIRL_3D_Game/Tools/Unreal/import_world_map.py"
Or headless:     UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="C:/Dev/CountriesIRL_3D_Game/Tools/Unreal/import_world_map.py"

Creates /Game/CrownsAndCommoners/UI/Map/T_WorldMap_England1455 (UI texture, BC7 so the lettering stays crisp, no mipmaps,
never streamed) and DA_WorldMap_England1455 (CIRLMapDefinition). Safe to re-run after redrawing the map.
"""
import json
import unreal

SOURCE = "C:/Dev/CountriesIRL_3D_Game/Saved/MapExport/T_WorldMap_England1455.png"
FOLDER = "/Game/CrownsAndCommoners/UI/Map"
TEXTURE = "T_WorldMap_England1455"
MARKER = "T_WorldMap_PlayerMarker"
ASSET = "DA_WorldMap_England1455"

cal = json.load(open(SOURCE.replace(".png", ".json"), encoding="utf-8"))

task = unreal.AssetImportTask()
task.filename = SOURCE
task.destination_path = FOLDER
task.destination_name = TEXTURE
task.automated = True
task.replace_existing = True
task.save = False
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

texture = unreal.load_asset("%s/%s" % (FOLDER, TEXTURE))
texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_BC7)
texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
texture.set_editor_property("never_stream", True)
texture.set_editor_property("srgb", True)
texture.set_editor_property("address_x", unreal.TextureAddress.TA_CLAMP)
texture.set_editor_property("address_y", unreal.TextureAddress.TA_CLAMP)
unreal.EditorAssetLibrary.save_asset("%s/%s" % (FOLDER, TEXTURE), False)

# The player's marker: small, uncompressed so its outline stays clean
task = unreal.AssetImportTask()
task.filename = SOURCE.replace(TEXTURE, MARKER)
task.destination_path = FOLDER
task.destination_name = MARKER
task.automated = True
task.replace_existing = True
task.save = False
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
marker = unreal.load_asset("%s/%s" % (FOLDER, MARKER))
marker.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
marker.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
marker.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
marker.set_editor_property("never_stream", True)
unreal.EditorAssetLibrary.save_asset("%s/%s" % (FOLDER, MARKER), False)

path = "%s/%s" % (FOLDER, ASSET)
asset = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None
if not asset:
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.CIRLMapDefinition)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(ASSET, FOLDER, unreal.CIRLMapDefinition, factory)
asset.set_editor_property("texture", texture)
asset.set_editor_property("player_marker", marker)
asset.set_editor_property("aspect", cal["aspect"])
asset.set_editor_property("origin_uv", unreal.Vector2D(cal["origin_uv"][0], cal["origin_uv"][1]))
asset.set_editor_property("uv_per_game_km", unreal.Vector2D(cal["uv_per_game_km"][0], cal["uv_per_game_km"][1]))
unreal.EditorAssetLibrary.save_loaded_asset(asset, False)
print("WORLDMAP_IMPORT_OK", texture.blueprint_get_size_x(), texture.blueprint_get_size_y(), cal)
