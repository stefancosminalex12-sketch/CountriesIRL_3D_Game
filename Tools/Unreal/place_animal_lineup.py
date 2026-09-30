"""
Place every imported animal in the dev sandbox, playing its idle animation (DevTest/Animals).

    py "C:/Dev/CountriesIRL_3D_Game/Tools/Unreal/place_animal_lineup.py"

Safe to re-run: the lineup is replaced.
"""
import unreal

ROOT = "/Game/CrownsAndCommoners/Animals"
FOLDER = "DevTest/Animals"
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

for actor in actors.get_all_level_actors():
    if str(actor.get_folder_path()) == FOLDER:
        actors.destroy_actor(actor)

# Rows in front of the season garden (south side), animals side-on to someone standing north of them
rows = [
    (-900, ["Horse", "Horse_White", "Donkey", "Deer"]),
    (-1150, ["Cow", "Bull", "Stag"]),
    (-700, ["Wolf", "Fox", "Husky", "ShibaInu"]),
]
for x, names in rows:
    meshes = [unreal.load_asset("%s/%s/%s" % (ROOT, name, name)) for name in names]
    lengths = [mesh.get_bounds().box_extent.y * 2 for mesh in meshes]
    gap = 70
    y = -(sum(lengths) + gap * (len(names) - 1)) / 2
    for name, mesh, length in zip(names, meshes, lengths):
        # The models' pivot isn't at the feet: lift each so its lowest point touches the ground
        bounds = mesh.get_bounds()
        lift = bounds.box_extent.z - bounds.origin.z
        actor = actors.spawn_actor_from_object(mesh, unreal.Vector(x, y + length / 2, lift), unreal.Rotator(0, 0, 0))
        actor.set_actor_label("Animal_" + name)
        actor.set_folder_path(FOLDER)
        idle = unreal.load_asset("%s/%s/%sAnimalArmature_Idle" % (ROOT, name, name))
        component = actor.skeletal_mesh_component
        component.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
        component.override_animation_data(idle, True, True, 0.0, 1.0)
        y += length + gap
        print("ANIMAL placed", name, "idle" if idle else "NO IDLE")

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
print("ANIMAL lineup done")
