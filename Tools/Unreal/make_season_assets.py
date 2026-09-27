"""
Create the season assets and a small test garden in the dev sandbox.

    py "C:/Dev/CountriesIRL_3D_Game/Tools/Unreal/make_season_assets.py"

- DA_Climate_Yorkshire   climate data (defaults come from the C++ class: Yorkshire, 1450s)
- MPC_Season             material parameters the season system writes every frame
- M_Seasonal_Ground      grass that follows GrassTint and turns white with Frost
- M_Seasonal_Leaves      foliage that follows LeafTint and thins out with LeafAmount (leaf fall)
- MI_Bark                plain bark colour for test trees
- DevTest/SeasonGarden   a grass patch with three simple trees in L_DevSandbox (placeholders until Fab trees)

Safe to re-run: assets are rebuilt and the garden is replaced.
"""
import unreal

FOLDER = "/Game/CountriesIRL/World"
tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary
assets = unreal.EditorAssetLibrary


def fresh(name, asset_class, factory):
    path = FOLDER + "/" + name
    if assets.does_asset_exist(path):
        assets.delete_asset(path)
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


existing = {str(p.get_editor_property("parameter_name")) for p in mpc.get_editor_property("scalar_parameters")}
if "LeafAmount" not in existing:
    mpc.set_editor_property("scalar_parameters", [
        scalar("LeafAmount", 0.65), scalar("Frost", 0.0), scalar("Mist", 0.0), scalar("Temperature", 11.0)])
    mpc.set_editor_property("vector_parameters", [
        vector("LeafTint", unreal.LinearColor(0.2, 0.43, 0.04, 1.0)),
        vector("GrassTint", unreal.LinearColor(0.12, 0.34, 0.035, 1.0))])


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
mel.connect_material_property(ground_colour, "", unreal.MaterialProperty.MP_BASE_COLOR)
rough = mel.create_material_expression(ground, unreal.MaterialExpressionConstant, -300, 250)
rough.set_editor_property("r", 0.95)
mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
mel.recompile_material(ground)

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
    place(name + "BranchL", cylinder, bark, unreal.Vector(x, y - 45 * size, 300 * size), unreal.Vector(0.12 * size, 0.12 * size, 1.3 * size), unreal.Rotator(0, 0, -35))
    place(name + "BranchR", cylinder, bark, unreal.Vector(x + 10, y + 45 * size, 310 * size), unreal.Vector(0.12 * size, 0.12 * size, 1.2 * size), unreal.Rotator(0, 0, 35))
    place(name + "BranchB", cylinder, bark, unreal.Vector(x - 40 * size, y, 320 * size), unreal.Vector(0.1 * size, 0.1 * size, 1.1 * size), unreal.Rotator(-30, 0, 0))
    # Canopy: three overlapping leaf balls
    place(name + "Canopy", sphere, leaves, unreal.Vector(x, y, 360 * size), unreal.Vector(2.6 * size, 2.6 * size, 2.2 * size))
    place(name + "CanopyL", sphere, leaves, unreal.Vector(x + 20, y - 85 * size, 320 * size), unreal.Vector(1.8 * size, 1.8 * size, 1.6 * size))
    place(name + "CanopyR", sphere, leaves, unreal.Vector(x - 20, y + 85 * size, 330 * size), unreal.Vector(1.8 * size, 1.8 * size, 1.6 * size))

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
print("SEASON assets and garden done")
