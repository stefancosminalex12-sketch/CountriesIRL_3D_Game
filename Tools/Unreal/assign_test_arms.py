"""
Give the test balls in the open level different coats of arms (so the heraldry can be seen in the world).
    py "C:/Dev/CrownsAndCommoners/Tools/Unreal/assign_test_arms.py"
"""
import unreal

ARMS = ["neville", "percy", "mowbray", "york", "talbot", "lancaster", "stafford", "courtenay", "clifford"]
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
balls = sorted([a for a in actors if a.get_class().get_name() == "BallCharacter"], key=lambda a: a.get_actor_location().y)
for index, ball in enumerate(balls):
    name = ARMS[index % len(ARMS)]
    texture = unreal.load_asset(f"/Game/CrownsAndCommoners/Characters/Flags/T_flag_{name}")
    ball.set_editor_property("flag", texture)
    ball.modify()
    print("TEST_ARMS", ball.get_actor_label(), name)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
