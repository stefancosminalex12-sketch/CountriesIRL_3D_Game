"""
Dev sandbox: remove the raised platform with ramps in the middle of the playground (template leftover; the rows of
test balls stood half inside its ramps) and put the player start on the ground. Safe to run again.
    headless: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this file>"
"""
import unreal

# Everything of the template playground this close to the middle belongs to the platform (the outer walls and corner
# blocks are further out)
REACH = 800.0

unreal.EditorLoadingAndSavingUtils.load_map("/Game/CrownsAndCommoners/Maps/L_DevSandbox")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

import os

report = []
files = []
for actor in actors.get_all_level_actors():
    where = actor.get_actor_location()
    if (isinstance(actor, unreal.StaticMeshActor) and str(actor.get_folder_path()) == "Playground"
            and actor.get_actor_label() != "Floor" and abs(where.x) < REACH and abs(where.y) < REACH):
        # Each placed object is its own small file (One File Per Actor)
        package = actor.get_package().get_path_name()
        files.append(unreal.Paths.convert_relative_path_to_full(
            unreal.Paths.project_content_dir() + package[len("/Game/"):] + ".uasset"))
        report.append("removed " + actor.get_actor_label())
        actors.destroy_actor(actor)

for actor in actors.get_all_level_actors():
    if isinstance(actor, unreal.PlayerStart):
        where = actor.get_actor_location()
        actor.set_actor_location(unreal.Vector(where.x, where.y, 100.0), False, False)
        actor.modify()

saved = unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
# Run headless, saving leaves the files of removed objects behind: delete them
for path in files:
    if os.path.exists(path):
        os.remove(path)
        report.append("deleted file " + path)
report.append("parts removed %d, saved %s" % (len(files), saved))
with open(unreal.Paths.project_saved_dir() + "ClaudeTmp/platform.txt", "w") as out:
    out.write(chr(10).join(report))
