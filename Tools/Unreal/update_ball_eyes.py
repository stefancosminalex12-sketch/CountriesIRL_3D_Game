"""
Eye material (M_BallEyes): adds eyelashes as an option. Works on the code that is really in the material's Custom
node: inserts the eyelash block (once) and adds the "Lashes" input (a scalar parameter, 0 = none, 1 = eyelashes),
then writes the node's full code to Docs/Shaders/BallEyes.hlsl so that file matches the material. Safe to run again.
    headless: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this file>"
Run with the game and editor closed (a material that is in use must not be rebuilt).
"""
import unreal

PROJECT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
lib = unreal.MaterialEditingLibrary
path = "/Game/CrownsAndCommoners/Characters/Materials/M_BallEyes"
m = unreal.load_asset(path)

# The Custom node feeds the opacity mask (through a component mask)
custom = None
node = lib.get_material_property_input_node(m, unreal.MaterialProperty.MP_OPACITY_MASK)
seen = 0
while node and seen < 8:
    if isinstance(node, unreal.MaterialExpressionCustom):
        custom = node
        break
    inputs = lib.get_inputs_for_material_expression(m, node)
    node = inputs[0] if inputs else None
    seen += 1
assert custom, "Custom node not found"

LASHES = '''    // Eyelashes (Lashes = 1): three short strokes fanning out from the outer top corner of the eye.
    // They start on the eye's edge (or on the upper lid when it is lowered, so they follow a blink)
    if (Lashes > 0.5)
    {
        float2 qo = float2(q.x * s, q.y);   // x runs toward the outer side of the face
        float lidTop = (UpperLid > 0.01) ? ((H + O) - UpperLid * 2.0 * (H + O)) : 1000.0;
        for (int k = 0; k < 3; k++)
        {
            float t = radians(18.0 + 26.0 * k);
            float2 lashRoot = float2(W * cos(t), min(H * sin(t), lidTop));
            float2 lashDir = normalize(float2(cos(t) * 1.3, sin(t) * 0.8 + 0.3));
            float2 rel = qo - lashRoot;
            float along = clamp(dot(rel, lashDir), 0.0, 0.55 * W);
            if (length(rel - lashDir * along) < 0.95) { mask = 1.0; white = 0.0; }
        }
    }

'''
MARKER = "    // Approximate signed distance to the eye ellipse (cm)"

code = str(custom.get_editor_property("code")).replace("\r\n", "\n")
print("EYES code length", len(code), "has lashes", "Lashes > 0.5" in code, "has marker", MARKER in code)
if "Lashes > 0.5" not in code:
    assert MARKER in code, "The place to insert the eyelashes was not found; nothing changed"
    code = code.replace(MARKER, LASHES + MARKER, 1)
    custom.set_editor_property("code", code)

names = [str(i.get_editor_property("input_name")) for i in custom.get_editor_property("inputs")]
if "Lashes" not in names:
    inputs = list(custom.get_editor_property("inputs"))
    lashes_input = unreal.CustomInput()
    lashes_input.set_editor_property("input_name", "Lashes")
    inputs.append(lashes_input)
    custom.set_editor_property("inputs", inputs)
    param = lib.create_material_expression(m, unreal.MaterialExpressionScalarParameter, -900, 700)
    param.set_editor_property("parameter_name", "Lashes")
    param.set_editor_property("default_value", 0.0)
    print("EYES connected Lashes", lib.connect_material_expressions(param, "", custom, "Lashes"))

print("EYES inputs", [str(i.get_editor_property("input_name")) for i in custom.get_editor_property("inputs")])
print("EYES connections", [(c.get_class().get_name()[18:] if c else None) for c in lib.get_inputs_for_material_expression(m, custom)])
lib.recompile_material(m)
unreal.EditorAssetLibrary.save_asset(path, False)

# Keep the documented copy identical to what the material really runs
with open(PROJECT + "Docs/Shaders/BallEyes.hlsl", "w", encoding="utf-8", newline="\n") as f:
    f.write("// The code of the Custom node in /Game/CrownsAndCommoners/Characters/Materials/M_BallEyes, written out by\n"
            "// Tools/Unreal/update_ball_eyes.py (this header is not in the node). Draws both countryball eyes on a sphere shell.\n"
            "// Inputs: P (local position on the shell), EyeScale, UpperLid, UpperLidAngle, LowerLid,\n"
            "//         Dead (0 alive, 1 x_x, 2 hollow skull sockets), Stretch (the shell's egg stretch), Lashes (0 / 1).\n"
            "// Output: float2(opacity mask, whiteness) -> x drives Opacity Mask, y drives Base Color.\n\n")
    f.write(str(custom.get_editor_property("code")).replace("\r\n", "\n"))
print("EYES done")
