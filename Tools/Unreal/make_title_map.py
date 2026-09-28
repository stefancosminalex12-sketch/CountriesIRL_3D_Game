"""
Create the title-screen level L_MainMenu (empty; its GameMode Override shows the title screen). Run inside the editor:
    py "C:/Dev/CountriesIRL_3D_Game/Tools/Unreal/make_title_map.py"
Afterwards the editor goes back to L_DevSandbox.
"""
import unreal

MAP = "/Game/CountriesIRL/Maps/L_MainMenu"
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

if not unreal.EditorAssetLibrary.does_asset_exist(MAP):
    levels.new_level(MAP, False)
else:
    levels.load_level(MAP)

world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
settings = world.get_world_settings()
settings.set_editor_property("default_game_mode", unreal.load_class(None, "/Script/CountriesIRL_3D_Game.CIRLTitleGameMode"))
levels.save_current_level()
print("TITLE_MAP", MAP, settings.get_editor_property("default_game_mode"))
levels.load_level("/Game/CountriesIRL/Maps/L_DevSandbox")
