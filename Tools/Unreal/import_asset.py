"""
Import a source file (FBX/glTF/GLB...) into the Unreal project. Run inside the editor:
    py "C:/Dev/CrownsAndCommoners/Tools/Unreal/import_asset.py" <source file> <destination folder, e.g. /Game/Characters>
"""
import sys
import unreal

source, destination = sys.argv[1], sys.argv[2]
task = unreal.AssetImportTask()
task.filename = source
task.destination_path = destination
task.automated = True
task.replace_existing = True
task.save = True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
print("IMPORT_RESULT", source, "->", [str(p) for p in task.imported_object_paths])
