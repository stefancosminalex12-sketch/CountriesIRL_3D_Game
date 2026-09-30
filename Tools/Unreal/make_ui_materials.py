"""
Create UI materials used by the menus. Run inside the editor:
    py "C:/Dev/CrownsAndCommoners/Tools/Unreal/make_ui_materials.py"

M_UI_PaperDoll: shows the paper-doll camera's picture (render target in the "Picture" parameter) with the
right transparency: scene captures store opacity inverted in alpha, so Opacity = 1 - alpha.
"""
import unreal

PATH = "/Game/CrownsAndCommoners/UI/Materials"
NAME = "M_UI_PaperDoll"
lib = unreal.MaterialEditingLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()

full = PATH + "/" + NAME
material = unreal.load_asset(full) if unreal.EditorAssetLibrary.does_asset_exist(full) else \
    tools.create_asset(NAME, PATH, unreal.Material, unreal.MaterialFactoryNew())
lib.delete_all_material_expressions(material)

material.set_editor_property("material_domain", unreal.MaterialDomain.MD_UI)
material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)

picture = lib.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -500, 0)
picture.set_editor_property("parameter_name", "Picture")
picture.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
one_minus = lib.create_material_expression(material, unreal.MaterialExpressionOneMinus, -200, 150)
# The capture's colours arrive gamma-encoded once too often (they look washed out): undo it
gamma = lib.create_material_expression(material, unreal.MaterialExpressionPower, -200, -50)
gamma.set_editor_property("const_exponent", 2.2)

lib.connect_material_expressions(picture, "RGB", gamma, "Base")
lib.connect_material_property(gamma, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
lib.connect_material_expressions(picture, "A", one_minus, "")
lib.connect_material_property(one_minus, "", unreal.MaterialProperty.MP_OPACITY)

lib.recompile_material(material)
unreal.EditorAssetLibrary.save_asset(full, False)
print("UI_MATERIAL", full)
