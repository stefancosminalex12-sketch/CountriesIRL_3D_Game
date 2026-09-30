"""
Import the ground textures (Art/Textures/tex_*.png, made seamless by Tools/make_tileable.py) and build M_Landscape,
the material the landscape is painted with.
    headless: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this file>"

Textures -> /Game/CrownsAndCommoners/World/Textures/T_tex_<name> (repeating, with mipmaps).
M_Landscape (/Game/CrownsAndCommoners/World/Materials) has one paint layer per ground: Grass, Meadow... (LAYERS below).
Each layer's texture repeats every few metres; a very large, soft brightness variation breaks up the repetition.
On top, the season system's weather (MPC_Season): wet ground is darker and shinier, frost and lying snow whiten it.

Only creates the material the first time. To change it, delete the asset with the editor closed and run this again
(rebuilding a material the game has loaded crashes the editor).
"""
import glob
import os
import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
TEXTURES = "/Game/CrownsAndCommoners/World/Textures"
MATERIALS = "/Game/CrownsAndCommoners/World/Materials"
tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary
assets = unreal.EditorAssetLibrary
report = []

# --- Textures ----------------------------------------------------------------------------------
tasks = []
for path in sorted(glob.glob(os.path.join(ROOT, "Art/Textures/tex_*.png"))):
    task = unreal.AssetImportTask()
    task.filename = path.replace("\\", "/")
    task.destination_path = TEXTURES
    task.destination_name = "T_" + os.path.splitext(os.path.basename(path))[0]
    task.automated = True
    task.replace_existing = True
    task.save = False
    tasks.append(task)
tools.import_asset_tasks(tasks)
for task in tasks:
    for object_path in task.imported_object_paths:
        texture = unreal.load_asset(object_path)
        texture.set_editor_property("address_x", unreal.TextureAddress.TA_WRAP)
        texture.set_editor_property("address_y", unreal.TextureAddress.TA_WRAP)
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_WORLD)
        assets.save_asset(object_path.split(".")[0], False)
        report.append("texture " + object_path)

# --- Material ----------------------------------------------------------------------------------
# (layer name, texture, metres before it repeats)
LAYERS = [
    ("Grass", "tex_grass", 4.0),
    ("Mud", "tex_mud_bank", 4.0),
    ("Road", "tex_dirt_road", 4.0),
    ("Forest", "tex_forest_floor", 5.0),
    ("Field", "tex_ploughed_field", 6.0),
    ("Rock", "tex_limestone_rock", 8.0),
    ("Cobbles", "tex_cobbles", 3.0),
]

path = MATERIALS + "/M_Landscape"
if assets.does_asset_exist(path):
    report.append("M_Landscape already exists: delete it with the editor closed to rebuild it")
else:
    material = tools.create_asset("M_Landscape", MATERIALS, unreal.Material, unreal.MaterialFactoryNew())
    mpc = unreal.load_asset("/Game/CrownsAndCommoners/World/MPC_Season")

    blend = mel.create_material_expression(material, unreal.MaterialExpressionLandscapeLayerBlend, -500, 0)
    inputs = []
    for name, _, _ in LAYERS:
        layer = unreal.LayerBlendInput()
        layer.set_editor_property("layer_name", name)
        layer.set_editor_property("blend_type", unreal.LandscapeLayerBlendType.LB_WEIGHT_BLEND)
        layer.set_editor_property("preview_weight", 1.0 if name == "Grass" else 0.0)
        inputs.append(layer)
    blend.set_editor_property("layers", inputs)

    coords = {}
    for index, (name, texture_name, metres) in enumerate(LAYERS):
        y = index * 220 - 700
        if metres not in coords:
            # One landscape quad is one metre
            node = mel.create_material_expression(material, unreal.MaterialExpressionLandscapeLayerCoords, -1300, y)
            node.set_editor_property("mapping_scale", metres)
            coords[metres] = node
        sample = mel.create_material_expression(material, unreal.MaterialExpressionTextureSample, -900, y)
        sample.set_editor_property("texture", unreal.load_asset(TEXTURES + "/T_" + texture_name))
        mel.connect_material_expressions(coords[metres], "", sample, "UVs")
        if not mel.connect_material_expressions(sample, "RGB", blend, "Layer " + name):
            report.append("could not connect layer " + name)

    def collection(name, x, y):
        node = mel.create_material_expression(material, unreal.MaterialExpressionCollectionParameter, x, y)
        node.set_editor_property("collection", mpc)
        node.set_editor_property("parameter_name", name)
        return node

    # Large soft light and dark patches (about 40 m across) so the repeats don't line up into a grid
    patches = mel.create_material_expression(material, unreal.MaterialExpressionNoise, -500, 350)
    patches.set_editor_property("scale", 0.0025)
    patches.set_editor_property("output_min", 0.82)
    patches.set_editor_property("output_max", 1.12)
    patches.set_editor_property("levels", 2)
    varied = mel.create_material_expression(material, unreal.MaterialExpressionMultiply, -250, 100)
    mel.connect_material_expressions(blend, "", varied, "A")
    mel.connect_material_expressions(patches, "", varied, "B")

    # Weather: frost, wetness, snow (the same as the other ground materials)
    frost_colour = mel.create_material_expression(material, unreal.MaterialExpressionConstant3Vector, -250, 300)
    frost_colour.set_editor_property("constant", unreal.LinearColor(0.62, 0.68, 0.74, 1.0))
    frost = collection("Frost", -500, 550)
    frost_strength = mel.create_material_expression(material, unreal.MaterialExpressionMultiply, -250, 550)
    frost_strength.set_editor_property("const_b", 0.6)
    mel.connect_material_expressions(frost, "", frost_strength, "A")
    frosted = mel.create_material_expression(material, unreal.MaterialExpressionLinearInterpolate, 0, 150)
    mel.connect_material_expressions(varied, "", frosted, "A")
    mel.connect_material_expressions(frost_colour, "", frosted, "B")
    mel.connect_material_expressions(frost_strength, "", frosted, "Alpha")

    wetness = collection("Wetness", 0, 500)
    darken = mel.create_material_expression(material, unreal.MaterialExpressionLinearInterpolate, 250, 450)
    darken.set_editor_property("const_a", 1.0)
    darken.set_editor_property("const_b", 0.6)
    mel.connect_material_expressions(wetness, "", darken, "Alpha")
    wet = mel.create_material_expression(material, unreal.MaterialExpressionMultiply, 250, 150)
    mel.connect_material_expressions(frosted, "", wet, "A")
    mel.connect_material_expressions(darken, "", wet, "B")

    snow_colour = mel.create_material_expression(material, unreal.MaterialExpressionConstant3Vector, 250, 300)
    snow_colour.set_editor_property("constant", unreal.LinearColor(0.86, 0.89, 0.94, 1.0))
    snow_cover = collection("SnowCover", 250, 650)
    final = mel.create_material_expression(material, unreal.MaterialExpressionLinearInterpolate, 500, 150)
    mel.connect_material_expressions(wet, "", final, "A")
    mel.connect_material_expressions(snow_colour, "", final, "B")
    mel.connect_material_expressions(snow_cover, "", final, "Alpha")
    mel.connect_material_property(final, "", unreal.MaterialProperty.MP_BASE_COLOR)

    rough = mel.create_material_expression(material, unreal.MaterialExpressionLinearInterpolate, 500, 450)
    rough.set_editor_property("const_a", 0.95)
    rough.set_editor_property("const_b", 0.35)
    mel.connect_material_expressions(wetness, "", rough, "Alpha")
    mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)

    mel.recompile_material(material)
    assets.save_asset(path, False)
    report.append("material " + path + " with layers " + ", ".join(n for n, _, _ in LAYERS))

with open(unreal.Paths.project_saved_dir() + "make_landscape_material.txt", "w") as out:
    out.write(chr(10).join(report))
