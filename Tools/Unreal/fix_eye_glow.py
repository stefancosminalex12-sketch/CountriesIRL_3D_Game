"""
Eyes: no glow and no glassy shine (user, 2026-09-30: in dark places and next to walls the eyes lit up and reflected,
and their glow lit the wall). Turns M_BallEyes' emissive strength to 0 and makes the surface matte like paint.
    headless: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="C:/Dev/CrownsAndCommoners/Tools/Unreal/fix_eye_glow.py"
Only changes values (no nodes are removed), so it is safe to run again.
"""
import unreal

lib = unreal.MaterialEditingLibrary
path = "/Game/CrownsAndCommoners/Characters/Materials/M_BallEyes"
m = unreal.load_asset(path)

# Emissive = eye mask x a constant: the constant becomes 0, so the eyes are lit only by the world like the ball
emissive = lib.get_material_property_input_node(m, unreal.MaterialProperty.MP_EMISSIVE_COLOR)
if isinstance(emissive, unreal.MaterialExpressionMultiply):
    print("EYEFIX emissive strength was", emissive.get_editor_property("const_b"))
    emissive.set_editor_property("const_b", 0.0)

# Matte like painted eyes, not glossy glass that mirrors the surroundings
rough = lib.get_material_property_input_node(m, unreal.MaterialProperty.MP_ROUGHNESS)
if isinstance(rough, unreal.MaterialExpressionConstant):
    print("EYEFIX roughness was", rough.get_editor_property("r"))
    rough.set_editor_property("r", 0.85)

lib.recompile_material(m)
unreal.EditorAssetLibrary.save_asset(path, False)
print("EYEFIX done")
