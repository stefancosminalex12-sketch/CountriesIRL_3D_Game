"""
Dev sandbox: spread the two rows of test balls out so the player can walk between them.
Men keep their row, women stand a row in front, half a place to the side. Safe to run again.
    headless: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this file>"
"""
import unreal

SPACING = 350.0       # between neighbours in a row (a ball is 136 wide)
MEN_X = 900.0
WOMEN_X = 500.0

unreal.EditorLoadingAndSavingUtils.load_map("/Game/CrownsAndCommoners/Maps/L_DevSandbox")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
balls = [a for a in actors.get_all_level_actors() if a.get_class().get_name() == "BallCharacter"]

report = []
for female, x, shift in ((False, MEN_X, 0.0), (True, WOMEN_X, SPACING * 0.5)):
    row = sorted([b for b in balls if b.get_editor_property("female") == female], key=lambda b: b.get_actor_location().y)
    start = -SPACING * (len(row) - 1) * 0.5 + shift
    for index, ball in enumerate(row):
        where = unreal.Vector(x, start + index * SPACING, ball.get_actor_location().z)
        ball.set_actor_location(where, False, False)
        ball.modify()
        report.append("%s -> %.0f %.0f" % (ball.get_actor_label(), where.x, where.y))

saved = unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
report.append("saved %s" % saved)
with open(unreal.Paths.project_saved_dir() + "space_sandbox_balls.txt", "w") as out:
    out.write(chr(10).join(report))
