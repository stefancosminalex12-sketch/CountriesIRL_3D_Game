# Crowns & Commoners

Unreal Engine 5.8 C++ project (project and module `CrownsAndCommoners`, content in `/Game/CrownsAndCommoners`, started from the Third Person template; renamed from CountriesIRL_3D_Game on 2026-09-30, class names keep their `CIRL` prefix).

- **Game design:** read `Docs/MASTER_FILE.md` before any design or gameplay work. It is the source of truth; keep it updated when decisions change.
- **Build (editor):** `"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" CrownsAndCommonersEditor Win64 Development "-Project=C:\Dev\CrownsAndCommoners\CrownsAndCommoners.uproject" -WaitMutex`
- **MCP:** Unreal MCP auto-starts with the editor (http://127.0.0.1:8000/mcp). If the chat's Unreal MCP connection failed (e.g. the chat started before the editor), talk to the editor directly with `python Tools/ue_mcp.py list | describe <toolset> | call <toolset> <tool> [json|@file.json] [--image out.png]` instead of waiting for a reconnect. Blender MCP needs Blender opened via the "Blender MCP" desktop shortcut (localhost:9876).
- Gameplay logic goes in C++; use Blueprints only for small visual hookups.
- **Build it like a proper game, not a one-shot prompt:** implement one focused system/feature at a time, with a clean architecture that later features can build on; compile each step, then launch the game for the user and tell them what to check (the user tests; Claude only verifies itself when the user can't easily see the result); commit each working step with a clear message; no giant all-at-once code dumps or throwaway hacks.
- World data (settlements, houses, characters, units, events, quests) should be data-driven so DLC regions can be added as new data + art.
- Large binary assets go through Git LFS (see `.gitattributes`).
- **Current work:** follow the Build Roadmap in `Docs/MASTER_FILE.md` (commoner origin first, v0.1 → v0.2 → v0.3).
- **Budget:** save money. Prefer **free** assets and tools wherever possible (Fab free, Quaternius/CC0, etc.); otherwise make things ourselves (Blender MCP, code). Only suggest paid assets when there is no good free option, and say so clearly.
- **Assets:** search Fab for free assets that fit, show them to the user and ask before using any; the user claims them with their Epic account (Claude cannot sign in).
- The user is a beginner: explain plainly, prefer doing things directly over click-by-click UI instructions.
