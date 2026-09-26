# CountriesIRL 3D Game

Unreal Engine 5.8 C++ project (module `CountriesIRL_3D_Game`, started from the Third Person template).

- **Game design:** read `Docs/MASTER_FILE.md` before any design or gameplay work. It is the source of truth; keep it updated when decisions change.
- **Build (editor):** `"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" CountriesIRL_3D_GameEditor Win64 Development "-Project=C:\Dev\CountriesIRL_3D_Game\CountriesIRL_3D_Game.uproject" -WaitMutex`
- **MCP:** Unreal MCP auto-starts with the editor (http://127.0.0.1:8000/mcp). Blender MCP needs Blender opened via the "Blender MCP" desktop shortcut (localhost:9876).
- Gameplay logic goes in C++; use Blueprints only for small visual hookups.
- World data (settlements, houses, characters, units, events, quests) should be data-driven so DLC regions can be added as new data + art.
- Large binary assets go through Git LFS (see `.gitattributes`).
- The user is a beginner: explain plainly, prefer doing things directly over click-by-click UI instructions.
