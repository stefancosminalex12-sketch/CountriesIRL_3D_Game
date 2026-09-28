"""
Type a console command into the running Unreal editor (the "Cmd" box at the bottom), via the Unreal MCP.
    python Tools/ue_console.py "py \"C:/Dev/CountriesIRL_3D_Game/Tools/Unreal/import_fonts.py\""
Useful for editor Python scripts, which the MCP tools can't run directly.
"""
import json
import re
import subprocess
import sys

SLATE = "SlateInspectorToolset.SlateInspectorToolset"


def call(tool, args):
    out = subprocess.run([sys.executable, "Tools/ue_mcp.py", "call", SLATE, tool, json.dumps(args)],
                         capture_output=True, text=True, check=True).stdout
    return json.loads(out).get("returnValue")


def main():
    command = sys.argv[1]
    call("Observe", {"ref": "", "maxDepth": 30})
    snapshot = call("Snapshot", {"ref": "", "maxDepth": 40, "bIncludeSourceLocations": False})
    # The console textbox is the first textbox after the "Cmd" label in the status bar
    match = re.search(r'text "Cmd".*?textbox \[[^\]]*\] \[ref=(tb\d+)\]', snapshot, re.S)
    if not match:
        sys.exit("Console box not found (is the editor minimized?)")
    call("Type", {"ref": match.group(1), "text": command, "submit": True})
    print("sent:", command)


if __name__ == "__main__":
    main()
