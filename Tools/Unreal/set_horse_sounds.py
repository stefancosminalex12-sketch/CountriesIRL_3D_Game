"""
Give the horses (DA_Mount_*) their sounds: hoofbeat loops per gait, neighs and snorts (Art/Audio/Sfx, made by
Tools/prepare_sfx.py). Safe to run again.
    headless: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this file>"
"""
import unreal

SFX = "/Game/CrownsAndCommoners/Audio/Sfx/"
load = lambda name: unreal.load_asset(SFX + name)
report = []
for asset in ("DA_Mount_Horse", "DA_Mount_HorseWhite"):
    mount = unreal.load_asset("/Game/CrownsAndCommoners/Animals/" + asset)
    mount.set_editor_property("walk_sound", load("sfx_horse_walk"))
    mount.set_editor_property("trot_sound", load("sfx_horse_trot"))
    mount.set_editor_property("canter_sound", load("sfx_horse_canter"))
    mount.set_editor_property("gallop_sound", load("sfx_horse_gallop"))
    mount.set_editor_property("neigh_sounds", [load("sfx_horse_neigh_%d" % i) for i in range(1, 6)])
    mount.set_editor_property("snort_sounds", [load("sfx_horse_snort_%d" % i) for i in range(1, 6)] + [load("sfx_horse_snort_deep")])
    unreal.EditorAssetLibrary.save_asset("/Game/CrownsAndCommoners/Animals/" + asset, False)
    report.append("%s: %d neighs, %d snorts" % (asset, len(mount.get_editor_property("neigh_sounds")), len(mount.get_editor_property("snort_sounds"))))
with open(unreal.Paths.project_saved_dir() + "set_horse_sounds.txt", "w") as out:
    out.write(chr(10).join(report))
