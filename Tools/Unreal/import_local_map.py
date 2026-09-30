"""
Import a town's detailed map (Art/UI/Maps/localmap_<town>.png, drawn by Tools/world/build_<town>_plan.py) and write
where it lies on the world map into its map data asset (DA_LocalMap_<Town>, a CIRLMapDefinition).
    headless: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this file>"

A local map covers TOWNS[name] = (game km east, game km north of its centre, size in game km). The Map tab fades it in
over the world map when you zoom in on it, and opens on it when you're inside it. List the asset in
Config/DefaultGame.ini > WorldSimulationSettings > +LocalMaps.
"""
import unreal

FOLDER = "/Game/CrownsAndCommoners/UI/Map"
ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
# Middleham's centre is the castle, which is where the dev sandbox is anchored on the world map (LevelMapAnchors)
TOWNS = {"Middleham": (6.4913, 20.72, 1.0)}

report = []
for town, (east, north, size) in TOWNS.items():
    texture_name = "T_LocalMap_" + town
    task = unreal.AssetImportTask()
    task.filename = ROOT + "Art/UI/Maps/localmap_%s.png" % town.lower()
    task.destination_path = FOLDER
    task.destination_name = texture_name
    task.automated = True
    task.replace_existing = True
    task.save = False
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    texture = unreal.load_asset("%s/%s" % (FOLDER, texture_name))
    texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_BC7)
    texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    texture.set_editor_property("never_stream", True)
    texture.set_editor_property("srgb", True)
    texture.set_editor_property("address_x", unreal.TextureAddress.TA_CLAMP)
    texture.set_editor_property("address_y", unreal.TextureAddress.TA_CLAMP)
    unreal.EditorAssetLibrary.save_asset("%s/%s" % (FOLDER, texture_name), False)

    asset_name = "DA_LocalMap_" + town
    path = "%s/%s" % (FOLDER, asset_name)
    map_class = unreal.load_class(None, "/Script/CrownsAndCommoners.CIRLMapDefinition")
    asset = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None
    if not asset:
        factory = unreal.DataAssetFactory()
        factory.set_editor_property("data_asset_class", map_class)
        asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, FOLDER, map_class, factory)
    # The picture spans [centre - size/2, centre + size/2] in both directions, north up
    asset.set_editor_property("texture", texture)
    asset.set_editor_property("aspect", 1.0)
    asset.set_editor_property("uv_per_game_km", unreal.Vector2D(1.0 / size, 1.0 / size))
    asset.set_editor_property("origin_uv", unreal.Vector2D(0.5 - east / size, 0.5 + north / size))
    unreal.EditorAssetLibrary.save_loaded_asset(asset, False)
    report.append("%s: %s, centre %.4f / %.4f game km, %.1f km across" % (town, path, east, north, size))

with open(unreal.Paths.project_saved_dir() + "import_local_map.txt", "w") as out:
    out.write(chr(10).join(report))
