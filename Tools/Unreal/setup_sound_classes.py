"""
Create the game's sound classes (safe to run again):
  /Game/CountriesIRL/Audio/Classes/SC_Master   the "Sound" volume
      SC_Music                                 music
      SC_SFX                                   everything else (Project Settings > Audio > Default Sound Class)
The player's volume sliders (C++ UCIRLAudioSubsystem) turn these up and down. Run inside the editor:
    py "C:/Dev/CountriesIRL_3D_Game/Tools/Unreal/setup_sound_classes.py"
or headless: UnrealEditor-Cmd.exe <project> -run=pythonscript -script="<this file>"
"""
import unreal

FOLDER = "/Game/CountriesIRL/Audio/Classes"
tools = unreal.AssetToolsHelpers.get_asset_tools()


def sound_class(name):
    path = f"{FOLDER}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    return tools.create_asset(name, FOLDER, unreal.SoundClass, unreal.SoundClassFactory())


master = sound_class("SC_Master")
music = sound_class("SC_Music")
sfx = sound_class("SC_SFX")
for child in (music, sfx):
    child.set_editor_property("parent_class", master)
master.set_editor_property("child_classes", [music, sfx])
for asset in (master, music, sfx):
    unreal.EditorAssetLibrary.save_loaded_asset(asset, False)
    print("SOUND_CLASS", asset.get_path_name())
