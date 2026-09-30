"""
Dev sandbox: two bandits to fight (folder DevTest/Bandits), standing at the north end behind the rows of test balls
and facing the player start. Safe to run again: the old ones are replaced. Close the game first (it locks the level).
    headless: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this file>"
"""
import os
import unreal

FOLDER = "DevTest/Bandits"
# (name, where, what it carries: empty = the bandit's own default, a padded jack, hood and cudgel)
BANDITS = [
    ("Bandit_Cudgel", unreal.Vector(1500.0, 300.0, 100.0), []),
    ("Bandit_Falchion", unreal.Vector(1500.0, 650.0, 100.0),
     ["tunic_plain_wool", "gambeson_padded_jack", "helmet_kettle_hat", "boots_ankle", "weapon_falchion", "offhand_buckler"]),
]

unreal.EditorLoadingAndSavingUtils.load_map("/Game/CrownsAndCommoners/Maps/L_DevSandbox")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
bandit_class = unreal.load_class(None, "/Script/CrownsAndCommoners.BanditCharacter")

report = []
old_files = []
for actor in actors.get_all_level_actors():
    if str(actor.get_folder_path()) == FOLDER:
        package = actor.get_package().get_path_name()
        old_files.append(unreal.Paths.convert_relative_path_to_full(
            unreal.Paths.project_content_dir() + package[len("/Game/"):] + ".uasset"))
        actors.destroy_actor(actor)

for name, where, gear in BANDITS:
    bandit = actors.spawn_actor_from_class(bandit_class, where, unreal.Rotator(roll=0.0, pitch=0.0, yaw=180.0))
    if gear:
        bandit.set_editor_property("starting_gear", [unreal.Name(item) for item in gear])
    bandit.set_folder_path(FOLDER)
    bandit.set_actor_label(name)
    report.append("placed %s with %s" % (name, [str(item) for item in bandit.get_editor_property("starting_gear")]))

saved = unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
# Run headless, saving leaves the files of removed objects behind: delete them
for path in old_files:
    if os.path.exists(path):
        os.remove(path)
report.append("saved %s" % saved)
with open(unreal.Paths.project_saved_dir() + "place_bandits.txt", "w") as out:
    out.write(chr(10).join(report))
