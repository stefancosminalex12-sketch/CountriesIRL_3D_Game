"""
Import game sounds (Art/Audio/<Folder>/*.wav, not Art/Audio/Source) as Sound Waves. Run inside the editor:
    py "C:/Dev/CrownsAndCommoners/Tools/Unreal/import_audio.py" [part of a file name]
or headless: UnrealEditor-Cmd.exe <project> -run=pythonscript -script="<this file> [part of a file name]"
Art/Audio/Ambience/amb_x.wav -> /Game/CrownsAndCommoners/Audio/Ambience/amb_x (ambiences are set to loop).
Art/Audio/Music/mus_x.wav -> /Game/CrownsAndCommoners/Audio/Music/mus_x (doesn't loop: playlists play it; sound class SC_Music so the Music slider
controls it; everything else uses the default class SC_SFX). Run setup_sound_classes.py first.
"""
import glob
import os
import sys
import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()) + "Art/Audio"
only = sys.argv[1] if len(sys.argv) > 1 else ""

tasks = []
for path in sorted(glob.glob(os.path.join(ROOT, "*", "*.wav"))):
    folder = os.path.basename(os.path.dirname(path))
    if folder == "Source" or only not in os.path.basename(path):
        continue
    task = unreal.AssetImportTask()
    task.filename = path.replace("\\", "/")
    task.destination_path = "/Game/CrownsAndCommoners/Audio/" + folder
    task.automated = True
    task.replace_existing = True
    task.save = False
    tasks.append(task)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
music_class = unreal.load_asset("/Game/CrownsAndCommoners/Audio/Classes/SC_Music")
for task in tasks:
    for object_path in task.imported_object_paths:
        sound = unreal.load_asset(object_path)
        if isinstance(sound, unreal.SoundWave) and "/Ambience/" in object_path:
            sound.set_editor_property("looping", True)
        if isinstance(sound, unreal.SoundWave) and "/Music/" in object_path:
            sound.set_editor_property("looping", False)   # playlists move on to the next track
            if music_class:
                sound.set_editor_property("sound_class_object", music_class)
        # New assets aren't always marked dirty after import: save regardless
        unreal.EditorAssetLibrary.save_asset(object_path.split(".")[0], False)
        print("IMPORT_RESULT", task.filename, "->", object_path)
