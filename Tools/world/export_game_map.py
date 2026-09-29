"""
Makes the game's copy of the planning map for the Esc menu's Map tab.

    python Tools/world/export_game_map.py

Reads Docs/World/plan_map_england_1455.png (+ .json calibration from draw_plan_map.py) and writes
Saved/MapExport/T_WorldMap_England1455.png: scaled to fit Unreal's 8192 px texture limit, both sides a multiple of 4
(block compression). Then import it in the editor with Tools/Unreal/import_world_map.py, which also writes the
calibration into the map's data asset. The calibration is in UV (0..1), so the texture's exact size doesn't matter.
"""
import json
import os
from PIL import Image

ROOT = "C:/Dev/CountriesIRL_3D_Game"
SOURCE = f"{ROOT}/Docs/World/plan_map_england_1455.png"
OUT_DIR = f"{ROOT}/Saved/MapExport"
MAX_SIDE = 8188

Image.MAX_IMAGE_PIXELS = None
img = Image.open(SOURCE).convert("RGB")
scale = min(1.0, MAX_SIDE / max(img.size))
size = tuple(max(4, int(round(side * scale / 4)) * 4) for side in img.size)
os.makedirs(OUT_DIR, exist_ok=True)
img.resize(size, Image.LANCZOS).save(f"{OUT_DIR}/T_WorldMap_England1455.png")

cal = json.load(open(SOURCE.replace(".png", ".json"), encoding="utf-8"))
W, H = cal["image_size"]
uv = {
    "origin_uv": [cal["origin_px"][0] / W, cal["origin_px"][1] / H],
    "uv_per_game_km": [cal["px_per_game_km"] / W, cal["px_per_game_km"] / H],
    "aspect": W / H,
    "texture_size": list(size),
}
json.dump(uv, open(f"{OUT_DIR}/T_WorldMap_England1455.json", "w"), indent=1)

# The player's marker (the user's icon, pointing up): trimmed to its outline and centred in a 72 px square (drawn ~34 px tall, so no mipmaps needed)
marker = Image.open(f"{ROOT}/Art/AI/MapIcons/map_player.png").convert("RGBA")
marker = marker.crop(marker.getchannel("A").getbbox())
side = max(marker.size)
square = Image.new("RGBA", (side, side), (0, 0, 0, 0))
square.paste(marker, ((side - marker.width) // 2, (side - marker.height) // 2))
square.resize((72, 72), Image.LANCZOS).save(f"{OUT_DIR}/T_WorldMap_PlayerMarker.png")
print("wrote", size, uv)
