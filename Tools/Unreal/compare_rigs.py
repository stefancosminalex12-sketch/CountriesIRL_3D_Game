"""Compare reference-pose bone transforms (world space) of two skeletal meshes. py compare_rigs.py /Game/A /Game/B"""
import sys
import unreal

BONES = ["pelvis", "spine_01", "spine_03", "neck_01", "head", "upperarm_l", "lowerarm_l", "hand_l", "thigh_l", "calf_l", "foot_l"]
poses = [unreal.AnimPoseExtensions.get_reference_pose(unreal.load_asset(path).skeleton) for path in sys.argv[1:3]]
for bone in BONES:
    row = []
    for pose in poses:
        t = unreal.AnimPoseExtensions.get_bone_pose(pose, bone, unreal.AnimPoseSpaces.WORLD)
        l, r = t.translation, t.rotation.rotator()
        row.append("loc(%.1f %.1f %.1f) rot(p%.0f y%.0f r%.0f)" % (l.x, l.y, l.z, r.pitch, r.yaw, r.roll))
    print("RIG", bone, " | ".join(row))
