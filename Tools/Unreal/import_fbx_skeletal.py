"""
Import animated FBX files as skeletal meshes with their animations.

    py "C:/Dev/CountriesIRL_3D_Game/Tools/Unreal/import_fbx_skeletal.py" <destination /Game/...> <file.fbx[@scale]> [more ...]

Each file goes into its own subfolder named after the file. "@0.4" imports the mesh and its
animations at 40% size (e.g. to bring a model to real-life scale).
"""
import os
import sys
import unreal

destination, files = sys.argv[1], sys.argv[2:]
tasks = []
for entry in files:
    path, _, scale = entry.partition("@")
    scale = float(scale) if scale else 1.0
    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_animations", True)
    options.set_editor_property("import_materials", True)
    options.set_editor_property("import_textures", True)
    options.set_editor_property("create_physics_asset", False)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    options.skeletal_mesh_import_data.set_editor_property("import_uniform_scale", scale)
    options.anim_sequence_import_data.set_editor_property("import_uniform_scale", scale)

    task = unreal.AssetImportTask()
    task.filename = path
    task.destination_path = destination + "/" + os.path.splitext(os.path.basename(path))[0]
    task.automated = True
    task.replace_existing = True
    task.save = True
    task.options = options
    tasks.append(task)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for task in tasks:
    meshes = [unreal.load_asset(p) for p in task.imported_object_paths]
    for asset in meshes:
        if isinstance(asset, unreal.SkeletalMesh):
            b = asset.get_bounds()
            print("IMPORTED mesh", asset.get_path_name(), "size", round(b.box_extent.x * 2), round(b.box_extent.y * 2), round(b.box_extent.z * 2))
    anims = [p for p in task.imported_object_paths if isinstance(unreal.load_asset(p), unreal.AnimSequence)]
    print("IMPORTED", os.path.basename(task.filename), len(task.imported_object_paths), "assets,", len(anims), "animations:", [p.split(".")[-1] for p in anims])
