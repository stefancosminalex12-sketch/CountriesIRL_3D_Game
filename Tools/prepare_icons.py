"""
Prepare item icons for the game: trim each icon in Art/AI/Icons to its visible pixels and centre it on a
512 x 512 transparent square with the same margin, so every icon fills its slot the same way.
    python Tools/prepare_icons.py
Writes Art/UI/Icons/icon_<name>.png (import with Tools/Unreal/import_ui_textures.py icon_).
"""
import glob
import os
from PIL import Image

SOURCE = "Art/AI/Icons"
OUT = "Art/UI/Icons"
SIZE = 512
MARGIN = 0.08   # empty border on each side, as a share of the square

os.makedirs(OUT, exist_ok=True)
for path in sorted(glob.glob(os.path.join(SOURCE, "*.png"))):
    icon = Image.open(path).convert("RGBA")
    # Ignore near-invisible specks when finding the edges
    box = icon.getchannel("A").point(lambda a: 255 if a > 8 else 0).getbbox()
    if not box:
        continue
    icon = icon.crop(box)
    inner = int(SIZE * (1 - 2 * MARGIN))
    scale = inner / max(icon.size)
    icon = icon.resize((max(1, round(icon.width * scale)), max(1, round(icon.height * scale))), Image.LANCZOS)
    square = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    square.alpha_composite(icon, ((SIZE - icon.width) // 2, (SIZE - icon.height) // 2))
    name = "icon_" + os.path.splitext(os.path.basename(path))[0]
    square.save(os.path.join(OUT, name + ".png"))
    print(name, icon.size)
