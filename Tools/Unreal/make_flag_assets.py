"""
Import the coats of arms (Art/Heraldry/flag_*.png) and build M_BallArms. Run inside the editor:
    py "C:/Dev/CrownsAndCommoners/Tools/Unreal/make_flag_assets.py"

M_BallArms paints a coat of arms across the ball: the texture is projected flat through the ball from the front
(object space, so it turns with the ball), like a countryball flag. Parameters:
  Flag (texture), Tint (colour) and TintAmount (0-1) for the damage flash and a rotting corpse.
The engine sphere is 100 units wide in its own space: U = 0.5 - Y/100, V = 0.5 - Z/100.
"""
import glob
import os
import unreal

SOURCE = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()) + "Art/Heraldry"
FLAGS = "/Game/CrownsAndCommoners/Characters/Flags"
MATERIALS = "/Game/CrownsAndCommoners/Characters/Materials"
lib = unreal.MaterialEditingLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()

tasks = []
for path in sorted(glob.glob(os.path.join(SOURCE, "flag_*.png"))):
    task = unreal.AssetImportTask()
    task.filename = path.replace("\\", "/")
    task.destination_path = FLAGS
    task.destination_name = "T_" + os.path.splitext(os.path.basename(path))[0]
    task.automated = True
    task.replace_existing = True
    tasks.append(task)
tools.import_asset_tasks(tasks)
for task in tasks:
    for object_path in task.imported_object_paths:
        texture = unreal.load_asset(object_path)
        texture.set_editor_property("address_x", unreal.TextureAddress.TA_CLAMP)
        texture.set_editor_property("address_y", unreal.TextureAddress.TA_CLAMP)
        unreal.EditorAssetLibrary.save_asset(object_path.split(".")[0], False)
        print("FLAG_TEXTURE", object_path)

# Only ever creates the material. Rebuilding a material the balls already have loaded crashes the editor
# (!IsRooted assert): to change it, delete the asset with the editor closed, then run this again.
full = MATERIALS + "/M_BallArms"
if unreal.EditorAssetLibrary.does_asset_exist(full):
    raise SystemExit("M_BallArms already exists: delete it with the editor closed to rebuild it")
material = tools.create_asset("M_BallArms", MATERIALS, unreal.Material, unreal.MaterialFactoryNew())

local = lib.create_material_expression(material, unreal.MaterialExpressionLocalPosition, -1300, 0)
def component_mask(x, y, channel):
    """Picks one channel; every channel is set explicitly (the defaults differ between engine versions)"""
    mask = lib.create_material_expression(material, unreal.MaterialExpressionComponentMask, x, y)
    for name in ("r", "g", "b", "a"):
        mask.set_editor_property(name, name == channel)
    return mask


mask_y = component_mask(-1100, -60, "g")
mask_z = component_mask(-1100, 80, "b")
lib.connect_material_expressions(local, "", mask_y, "")
lib.connect_material_expressions(local, "", mask_z, "")


def scale_offset(source, x, y):
    """value * -0.01 + 0.5"""
    mul = lib.create_material_expression(material, unreal.MaterialExpressionMultiply, x, y)
    mul.set_editor_property("const_b", -0.01)
    add = lib.create_material_expression(material, unreal.MaterialExpressionAdd, x + 150, y)
    add.set_editor_property("const_b", 0.5)
    lib.connect_material_expressions(source, "", mul, "A")
    lib.connect_material_expressions(mul, "", add, "A")
    return add


u = scale_offset(mask_y, -950, -60)
v = scale_offset(mask_z, -950, 80)
uv = lib.create_material_expression(material, unreal.MaterialExpressionAppendVector, -650, 0)
lib.connect_material_expressions(u, "", uv, "A")
lib.connect_material_expressions(v, "", uv, "B")

flag = lib.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -450, 0)
flag.set_editor_property("parameter_name", "Flag")
flag.set_editor_property("texture", unreal.load_asset(FLAGS + "/T_flag_england"))
lib.connect_material_expressions(uv, "", flag, "UVs")

tint = lib.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -450, 250)
tint.set_editor_property("parameter_name", "Tint")
tint.set_editor_property("default_value", unreal.LinearColor(1.0, 0.06, 0.04, 1.0))
amount = lib.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -450, 400)
amount.set_editor_property("parameter_name", "TintAmount")
amount.set_editor_property("default_value", 0.0)

lerp = lib.create_material_expression(material, unreal.MaterialExpressionLinearInterpolate, -150, 100)
lib.connect_material_expressions(flag, "RGB", lerp, "A")
lib.connect_material_expressions(tint, "", lerp, "B")
lib.connect_material_expressions(amount, "", lerp, "Alpha")
lib.connect_material_property(lerp, "", unreal.MaterialProperty.MP_BASE_COLOR)

rough = lib.create_material_expression(material, unreal.MaterialExpressionConstant, -150, 300)
rough.set_editor_property("r", 0.75)
lib.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)

lib.recompile_material(material)
unreal.EditorAssetLibrary.save_asset(full, False)
print("FLAG_MATERIAL", full)
