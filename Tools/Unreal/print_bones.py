"""Print a skeletal mesh's bone names and its bounds. py ".../print_bones.py" /Game/Path/To/Mesh"""
import sys
import unreal

mesh = unreal.load_asset(sys.argv[1])
pose = unreal.AnimPoseExtensions.get_reference_pose(mesh.skeleton)
bones = [str(name) for name in unreal.AnimPoseExtensions.get_bone_names(pose)]
bounds = mesh.get_bounds()
print("BONES", mesh.get_name(), len(bones), bones, "HEIGHT", round(bounds.origin.z + bounds.box_extent.z, 1))
