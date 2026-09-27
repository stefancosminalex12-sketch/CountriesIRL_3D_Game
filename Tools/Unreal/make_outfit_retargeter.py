"""
Build the IK Rigs + IK Retargeter that copy Manny's animation onto the Quaternius outfit rig.

    py "C:/Dev/CountriesIRL_3D_Game/Tools/Unreal/make_outfit_retargeter.py"

Safe to re-run: existing assets are replaced.
"""
import unreal

FOLDER = "/Game/CountriesIRL/Characters/Outfits/Retarget"
SOURCE_MESH = "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"
TARGET_MESH = "/Game/CountriesIRL/Characters/Outfits/MalePeasant/SKM_Outfit_MalePeasant/SkeletalMeshes/Male_Peasant_Body"

tools = unreal.AssetToolsHelpers.get_asset_tools()
SOURCE = unreal.RetargetSourceOrTarget.SOURCE
TARGET = unreal.RetargetSourceOrTarget.TARGET


def fresh_asset(name, asset_class, factory):
    path = FOLDER + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)
    return tools.create_asset(name, FOLDER, asset_class, factory)


def make_ik_rig(name, mesh_path):
    rig = fresh_asset(name, unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
    controller = unreal.IKRigController.get_controller(rig)
    controller.set_skeletal_mesh(unreal.load_asset(mesh_path))
    controller.apply_auto_generated_retarget_definition()
    controller.apply_auto_fbik()
    print("RETARGET rig", name, "root", controller.get_retarget_root(), "chains", [str(c.chain_name) for c in controller.get_retarget_chains()])
    return rig


source_rig = make_ik_rig("IK_Manny", SOURCE_MESH)
target_rig = make_ik_rig("IK_QuaterniusOutfit", TARGET_MESH)

retargeter = fresh_asset("RTG_Manny_To_QuaterniusOutfit", unreal.IKRetargeter, unreal.IKRetargetFactory())
controller = unreal.IKRetargeterController.get_controller(retargeter)
controller.set_ik_rig(SOURCE, source_rig)
controller.set_ik_rig(TARGET, target_rig)
controller.set_preview_mesh(SOURCE, unreal.load_asset(SOURCE_MESH))
controller.set_preview_mesh(TARGET, unreal.load_asset(TARGET_MESH))
controller.remove_all_ops()
controller.add_default_ops()
controller.assign_ik_rig_to_all_ops(SOURCE, source_rig)
controller.assign_ik_rig_to_all_ops(TARGET, target_rig)
controller.auto_map_chains(unreal.AutoMapChainType.FUZZY, True)

# Manny stands in an A-pose, the outfit rig in a T-pose: bend the outfit's retarget pose to match Manny
controller.create_retarget_pose("MatchManny", TARGET)
controller.auto_align_all_bones(TARGET, unreal.RetargetAutoAlignMethod.CHAIN_TO_CHAIN)

unreal.EditorAssetLibrary.save_directory(FOLDER)
print("RETARGET done ops", controller.get_num_retarget_ops(), "pose", controller.get_current_retarget_pose_name(TARGET))
