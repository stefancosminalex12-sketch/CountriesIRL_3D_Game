"""
Dev sandbox: a second row of balls, one woman (eyelashes) for each ball of the emotion line-up, standing in front of it.
Safe to run again: the old row (folder DevTest/Women) is replaced.
    headless: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this file>"
"""
import unreal

FOLDER = "DevTest/Women"
OFFSET = unreal.Vector(-300.0, 100.0, 0.0)   # in front of the men's row, half a place to the side

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/CrownsAndCommoners/Maps/L_DevSandbox")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ball_class = unreal.load_class(None, "/Script/CrownsAndCommoners.BallCharacter")

for actor in actors.get_all_level_actors():
    if str(actor.get_folder_path()) == FOLDER:
        actors.destroy_actor(actor)

men = [a for a in actors.get_all_level_actors() if a.get_class() == ball_class and not a.get_editor_property("female")]
made = 0
for man in men:
    # (duplicate_actor needs the editor window; a fresh ball with the same look works headless too)
    woman = actors.spawn_actor_from_class(ball_class, man.get_actor_location() + OFFSET, man.get_actor_rotation())
    if not woman:
        continue
    for prop in ("flag", "starting_emotion"):
        woman.set_editor_property(prop, man.get_editor_property(prop))
    woman.set_editor_property("female", True)
    woman.set_folder_path(FOLDER)
    woman.set_actor_label("Woman_" + str(man.get_editor_property("starting_emotion")).split(".")[-1].split(":")[0].title())
    made += 1

saved = unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
print("WOMEN placed", made, "of", len(men), "saved", saved)
