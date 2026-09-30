"""
Create the season assets and a small test garden in the dev sandbox.

    py "C:/Dev/CrownsAndCommoners/Tools/Unreal/make_season_assets.py"

- DA_Climate_Yorkshire   climate data (defaults come from the C++ class: Yorkshire, 1450s)
- MPC_Season             material parameters the season system writes every frame
- M_Seasonal_Ground      grass that follows GrassTint and turns white with Frost
- M_Seasonal_Leaves      foliage that follows LeafTint and thins out with LeafAmount (leaf fall)
- MI_Bark                plain bark colour for test trees
- DevTest/SeasonGarden   a grass patch with three simple trees in L_DevSandbox (placeholders until Fab trees)

Safe to re-run: assets are rebuilt and the garden is replaced.
"""
import unreal

FOLDER = "/Game/CrownsAndCommoners/World"
tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary
assets = unreal.EditorAssetLibrary


def fresh(name, asset_class, factory):
    """The asset, emptied so it can be rebuilt. Existing assets are reused (not deleted), so everything that
    already points at them (placed actors, C++ defaults) stays connected."""
    path = FOLDER + "/" + name
    if assets.does_asset_exist(path):
        asset = unreal.load_asset(path)
        if isinstance(asset, unreal.Material):
            mel.delete_all_material_expressions(asset)
        return asset
    return tools.create_asset(name, FOLDER, asset_class, factory)


# --- Climate data -------------------------------------------------------------------------------
if not assets.does_asset_exist(FOLDER + "/DA_Climate_Yorkshire"):
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.ClimateProfile)
    tools.create_asset("DA_Climate_Yorkshire", FOLDER, unreal.ClimateProfile, factory)

# --- Material parameter collection (defaults = mid May, so the editor shows spring) --------------
mpc_path = FOLDER + "/MPC_Season"
mpc = unreal.load_asset(mpc_path) if assets.does_asset_exist(mpc_path) else tools.create_asset(
    "MPC_Season", FOLDER, unreal.MaterialParameterCollection, unreal.MaterialParameterCollectionFactoryNew())


def scalar(name, value):
    p = unreal.CollectionScalarParameter()
    p.set_editor_property("parameter_name", name)
    p.set_editor_property("default_value", value)
    return p


def vector(name, value):
    p = unreal.CollectionVectorParameter()
    p.set_editor_property("parameter_name", name)
    p.set_editor_property("default_value", value)
    return p


# Add any missing parameters (existing ones keep their ids, so materials stay connected)
SCALARS = [("LeafAmount", 0.65), ("Frost", 0.0), ("Mist", 0.0), ("Temperature", 11.0),
           ("CloudCover", 0.5), ("Rain", 0.0), ("Snowfall", 0.0), ("Wetness", 0.0), ("SnowCover", 0.0), ("Wind", 4.0), ("Daylight", 1.0)]
VECTORS = [("LeafTint", unreal.LinearColor(0.2, 0.43, 0.04, 1.0)), ("GrassTint", unreal.LinearColor(0.12, 0.34, 0.035, 1.0))]
scalars = list(mpc.get_editor_property("scalar_parameters"))
have = {str(p.get_editor_property("parameter_name")) for p in scalars}
scalars += [scalar(n, v) for n, v in SCALARS if n not in have]
mpc.set_editor_property("scalar_parameters", scalars)
vectors = list(mpc.get_editor_property("vector_parameters"))
have = {str(p.get_editor_property("parameter_name")) for p in vectors}
vectors += [vector(n, v) for n, v in VECTORS if n not in have]
mpc.set_editor_property("vector_parameters", vectors)


def rgb(material, source, x, y):
    """Colour parameters come out as RGBA; keep RGB"""
    mask = mel.create_material_expression(material, unreal.MaterialExpressionComponentMask, x, y)
    for channel in ("r", "g", "b"):
        mask.set_editor_property(channel, True)
    assert mel.connect_material_expressions(source, "", mask, "")
    return mask


def collection_param(material, name, x, y):
    node = mel.create_material_expression(material, unreal.MaterialExpressionCollectionParameter, x, y)
    node.set_editor_property("collection", mpc)
    node.set_editor_property("parameter_name", name)
    return node


def noise(material, scale, low, high, x, y):
    node = mel.create_material_expression(material, unreal.MaterialExpressionNoise, x, y)
    node.set_editor_property("scale", scale)
    node.set_editor_property("output_min", low)
    node.set_editor_property("output_max", high)
    node.set_editor_property("levels", 2)
    return node


# --- Ground: grass colour with patchy variation, frost on top ------------------------------------
ground = fresh("M_Seasonal_Ground", unreal.Material, unreal.MaterialFactoryNew())
grass = collection_param(ground, "GrassTint", -900, -100)
patches = noise(ground, 0.004, 0.8, 1.12, -900, 50)
tinted = mel.create_material_expression(ground, unreal.MaterialExpressionMultiply, -600, 0)
assert mel.connect_material_expressions(rgb(ground, grass, -750, -100), "", tinted, "A")
mel.connect_material_expressions(patches, "", tinted, "B")
frost_colour = mel.create_material_expression(ground, unreal.MaterialExpressionConstant3Vector, -600, 150)
frost_colour.set_editor_property("constant", unreal.LinearColor(0.62, 0.68, 0.74, 1.0))
frost = collection_param(ground, "Frost", -900, 250)
frost_strength = mel.create_material_expression(ground, unreal.MaterialExpressionMultiply, -600, 280)
frost_strength.set_editor_property("const_b", 0.75)
mel.connect_material_expressions(frost, "", frost_strength, "A")
ground_colour = mel.create_material_expression(ground, unreal.MaterialExpressionLinearInterpolate, -300, 50)
mel.connect_material_expressions(tinted, "", ground_colour, "A")
mel.connect_material_expressions(frost_colour, "", ground_colour, "B")
mel.connect_material_expressions(frost_strength, "", ground_colour, "Alpha")
# Wet ground is darker and shinier; lying snow turns it white
wetness = collection_param(ground, "Wetness", -600, 400)
darken = mel.create_material_expression(ground, unreal.MaterialExpressionLinearInterpolate, -300, 400)
darken.set_editor_property("const_a", 1.0)
darken.set_editor_property("const_b", 0.55)
mel.connect_material_expressions(wetness, "", darken, "Alpha")
wet_colour = mel.create_material_expression(ground, unreal.MaterialExpressionMultiply, -100, 50)
mel.connect_material_expressions(ground_colour, "", wet_colour, "A")
mel.connect_material_expressions(darken, "", wet_colour, "B")
snow_colour = mel.create_material_expression(ground, unreal.MaterialExpressionConstant3Vector, -100, 200)
snow_colour.set_editor_property("constant", unreal.LinearColor(0.86, 0.89, 0.94, 1.0))
snow_cover = collection_param(ground, "SnowCover", -300, 550)
final_colour = mel.create_material_expression(ground, unreal.MaterialExpressionLinearInterpolate, 100, 50)
mel.connect_material_expressions(wet_colour, "", final_colour, "A")
mel.connect_material_expressions(snow_colour, "", final_colour, "B")
mel.connect_material_expressions(snow_cover, "", final_colour, "Alpha")
mel.connect_material_property(final_colour, "", unreal.MaterialProperty.MP_BASE_COLOR)
rough = mel.create_material_expression(ground, unreal.MaterialExpressionLinearInterpolate, 100, 400)
rough.set_editor_property("const_a", 0.95)
rough.set_editor_property("const_b", 0.3)
mel.connect_material_expressions(wetness, "", rough, "Alpha")
mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
mel.recompile_material(ground)

# --- Rain streaks (faint, see-through, lit so they are dark at night) and snowflakes ---------------
def glowing_drop(name, colour, opacity, x=-400):
    """Unlit drops that glow with the daylight (bright by day, faint at night), so they read as rain/snow
    instead of dark specks against the bright sky"""
    material = fresh(name, unreal.Material, unreal.MaterialFactoryNew())
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    # Drops are drawn as instanced meshes: without this flag the engine silently swaps in its default material
    material.set_editor_property("used_with_instanced_static_meshes", True)
    if opacity < 1.0:
        material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    tint = mel.create_material_expression(material, unreal.MaterialExpressionConstant3Vector, x, 0)
    tint.set_editor_property("constant", colour)
    daylight = collection_param(material, "Daylight", x, 200)
    level = mel.create_material_expression(material, unreal.MaterialExpressionLinearInterpolate, x + 200, 200)
    level.set_editor_property("const_a", 0.03)
    level.set_editor_property("const_b", 0.55)
    mel.connect_material_expressions(daylight, "", level, "Alpha")
    glow = mel.create_material_expression(material, unreal.MaterialExpressionMultiply, x + 400, 0)
    mel.connect_material_expressions(tint, "", glow, "A")
    mel.connect_material_expressions(level, "", glow, "B")
    mel.connect_material_property(glow, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    if opacity < 1.0:
        alpha = mel.create_material_expression(material, unreal.MaterialExpressionConstant, x + 400, 250)
        alpha.set_editor_property("r", opacity)
        mel.connect_material_property(alpha, "", unreal.MaterialProperty.MP_OPACITY)
    mel.recompile_material(material)
    return material


rain = glowing_drop("M_Rain", unreal.LinearColor(0.8, 0.85, 0.9, 1.0), 0.16)
snow = glowing_drop("M_Snow", unreal.LinearColor(1.0, 1.0, 1.0, 1.0), 1.0)

# --- Leaves: seasonal colour, clusters drop out as LeafAmount falls ------------------------------
leaves = fresh("M_Seasonal_Leaves", unreal.Material, unreal.MaterialFactoryNew())
leaves.set_editor_property("blend_mode", unreal.BlendMode.BLEND_MASKED)
leaves.set_editor_property("two_sided", True)
leaf_tint = collection_param(leaves, "LeafTint", -900, -100)
leaf_shade = noise(leaves, 0.03, 0.75, 1.15, -900, 50)
leaf_colour = mel.create_material_expression(leaves, unreal.MaterialExpressionMultiply, -600, 0)
assert mel.connect_material_expressions(rgb(leaves, leaf_tint, -750, -100), "", leaf_colour, "A")
mel.connect_material_expressions(leaf_shade, "", leaf_colour, "B")
mel.connect_material_property(leaf_colour, "", unreal.MaterialProperty.MP_BASE_COLOR)
# Mask = LeafAmount - clusterNoise + 1/3: a cluster shows while LeafAmount is above its noise value
amount = collection_param(leaves, "LeafAmount", -900, 250)
clusters = noise(leaves, 0.015, 0.02, 0.98, -900, 400)
remaining = mel.create_material_expression(leaves, unreal.MaterialExpressionSubtract, -600, 300)
mel.connect_material_expressions(amount, "", remaining, "A")
mel.connect_material_expressions(clusters, "", remaining, "B")
mask = mel.create_material_expression(leaves, unreal.MaterialExpressionAdd, -350, 300)
mask.set_editor_property("const_b", 0.3333)
mel.connect_material_expressions(remaining, "", mask, "A")
mel.connect_material_property(mask, "", unreal.MaterialProperty.MP_OPACITY_MASK)
leaf_rough = mel.create_material_expression(leaves, unreal.MaterialExpressionConstant, -350, 150)
leaf_rough.set_editor_property("r", 0.8)
mel.connect_material_property(leaf_rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
mel.recompile_material(leaves)

# --- Bark ----------------------------------------------------------------------------------------
bark = fresh("MI_Bark", unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
mel.set_material_instance_parent(bark, unreal.load_asset("/Engine/BasicShapes/BasicShapeMaterial"))
mel.set_material_instance_vector_parameter_value(bark, "Color", unreal.LinearColor(0.12, 0.07, 0.035, 1.0))

assets.save_directory(FOLDER)

# --- Test garden in the sandbox (south side, between the corner blocks) --------------------------
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
GARDEN = "DevTest/SeasonGarden"
for actor in actors.get_all_level_actors():
    if str(actor.get_folder_path()) == GARDEN:
        actors.destroy_actor(actor)

plane = unreal.load_asset("/Engine/BasicShapes/Plane")
cylinder = unreal.load_asset("/Engine/BasicShapes/Cylinder")
sphere = unreal.load_asset("/Engine/BasicShapes/Sphere")


def place(label, mesh, material, location, scale, rotation=unreal.Rotator(0, 0, 0)):
    actor = actors.spawn_actor_from_object(mesh, location, rotation)
    actor.set_actor_label(label)
    actor.set_actor_scale3d(scale)
    actor.set_folder_path(GARDEN)
    actor.static_mesh_component.set_material(0, material)
    return actor


place("Garden_Grass", plane, ground, unreal.Vector(-1450, 0, 1), unreal.Vector(14, 16, 1))
trees = [(-1500, -450, 1.0), (-1700, 150, 1.25), (-1300, 480, 0.85)]
for index, (x, y, size) in enumerate(trees):
    name = "Garden_Tree%d_" % (index + 1)
    place(name + "Trunk", cylinder, bark, unreal.Vector(x, y, 130 * size), unreal.Vector(0.32 * size, 0.32 * size, 2.6 * size))
    # Bare branches, visible once the leaves have fallen
    place(name + "BranchL", cylinder, bark, unreal.Vector(x, y - 45 * size, 300 * size), unreal.Vector(0.12 * size, 0.12 * size, 1.3 * size), unreal.Rotator(roll=-35, pitch=0, yaw=0))
    place(name + "BranchR", cylinder, bark, unreal.Vector(x + 10, y + 45 * size, 310 * size), unreal.Vector(0.12 * size, 0.12 * size, 1.2 * size), unreal.Rotator(roll=35, pitch=0, yaw=0))
    place(name + "BranchB", cylinder, bark, unreal.Vector(x - 40 * size, y, 320 * size), unreal.Vector(0.1 * size, 0.1 * size, 1.1 * size), unreal.Rotator(roll=0, pitch=-30, yaw=0))
    # Canopy: three overlapping leaf balls
    place(name + "Canopy", sphere, leaves, unreal.Vector(x, y, 360 * size), unreal.Vector(2.6 * size, 2.6 * size, 2.2 * size))
    place(name + "CanopyL", sphere, leaves, unreal.Vector(x + 20, y - 85 * size, 320 * size), unreal.Vector(1.8 * size, 1.8 * size, 1.6 * size))
    place(name + "CanopyR", sphere, leaves, unreal.Vector(x - 20, y + 85 * size, 330 * size), unreal.Vector(1.8 * size, 1.8 * size, 1.6 * size))

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
print("SEASON assets and garden done")
