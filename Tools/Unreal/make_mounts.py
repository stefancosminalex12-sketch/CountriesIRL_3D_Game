"""
Create the horse MountDefinition assets and place rideable horses in the dev sandbox.

    py "C:/Dev/CountriesIRL_3D_Game/Tools/Unreal/make_mounts.py"

Safe to re-run: assets are updated and the horses (DevTest/Horses) replaced.
"""
import unreal

ANIMALS = "/Game/CrownsAndCommoners/Animals"
tools = unreal.AssetToolsHelpers.get_asset_tools()


def mount_asset(name, model):
    path = "%s/%s" % (ANIMALS, name)
    asset = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None
    if not asset:
        factory = unreal.DataAssetFactory()
        factory.set_editor_property("data_asset_class", unreal.MountDefinition)
        asset = tools.create_asset(name, ANIMALS, unreal.MountDefinition, factory)
    base = "%s/%s/%sAnimalArmature_" % (ANIMALS, model, model)
    asset.set_editor_property("mesh", unreal.load_asset("%s/%s/%s" % (ANIMALS, model, model)))
    asset.set_editor_property("idle_anim", unreal.load_asset(base + "Idle"))
    asset.set_editor_property("walk_anim", unreal.load_asset(base + "Walk"))
    asset.set_editor_property("gallop_anim", unreal.load_asset(base + "Gallop"))
    asset.set_editor_property("jump_anim", unreal.load_asset(base + "Gallop_Jump"))
    asset.set_editor_property("death_anim", unreal.load_asset(base + "Death"))
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return asset


brown = mount_asset("DA_Mount_Horse", "Horse")
white = mount_asset("DA_Mount_HorseWhite", "Horse_White")

# Two horses near the player start, side-on so they're easy to walk up to
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
FOLDER = "DevTest/Horses"
horse_class = unreal.load_class(None, "/Script/CountriesIRL_3D_Game.Horse")
for actor in actors.get_all_level_actors():
    if str(actor.get_folder_path()) == FOLDER or actor.get_class() == horse_class:
        actors.destroy_actor(actor)

for label, definition, location in [("Horse_Brown", brown, unreal.Vector(1650, -500, 90)),
                                     ("Horse_White", white, unreal.Vector(1250, -500, 90))]:
    horse = actors.spawn_actor_from_class(horse_class, location, unreal.Rotator(roll=0, pitch=0, yaw=180))
    horse.set_editor_property("definition", definition)
    horse.set_actor_label(label)
    horse.set_folder_path(FOLDER)
    print("MOUNTS placed", label)

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
print("MOUNTS done")
