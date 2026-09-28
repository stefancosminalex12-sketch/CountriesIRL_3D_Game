"""
Import the UI fonts (Art/Fonts/*.ttf) as Font Face assets. Run inside the editor:
    py "C:/Dev/CountriesIRL_3D_Game/Tools/Unreal/import_fonts.py"
The game builds its fonts from these faces in code (see UI/CIRLUIStyle.cpp).
"""
import glob
import os
import unreal

SOURCE = "C:/Dev/CountriesIRL_3D_Game/Art/Fonts"
DESTINATION = "/Game/CountriesIRL/UI/Fonts"

tasks = []
for path in sorted(glob.glob(os.path.join(SOURCE, "*.ttf"))):
    task = unreal.AssetImportTask()
    task.filename = path.replace("\\", "/")
    task.destination_path = DESTINATION
    task.destination_name = "FF_" + os.path.splitext(os.path.basename(path))[0].replace("-", "_")
    task.automated = True
    task.replace_existing = True
    task.save = True
    tasks.append(task)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for task in tasks:
    print("IMPORT_RESULT", task.filename, "->", [str(p) for p in task.imported_object_paths])
