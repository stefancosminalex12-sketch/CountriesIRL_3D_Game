"""
Prepare item icons for the game: trim each icon in Art/AI/Icons to its visible pixels and centre it on a
512 x 512 transparent square with the same margin, so every icon fills its slot the same way.
    python Tools/prepare_icons.py
Writes Art/UI/Icons/icon_<name>.png (import with Tools/Unreal/import_ui_textures.py icon_).
Also the map markers (Art/AI/MapIcons/marker_*.png) for the compass bar: Art/UI/Markers/marker_<name>.png, 256 x 256.
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


def fit(path, size, margin):
    """Trims an image to its visible pixels and centres it on a transparent square"""
    image = Image.open(path).convert("RGBA")
    box = image.getchannel("A").point(lambda a: 255 if a > 8 else 0).getbbox()
    if not box:
        return None
    image = image.crop(box)
    inner = int(size * (1 - 2 * margin))
    scale = inner / max(image.size)
    image = image.resize((max(1, round(image.width * scale)), max(1, round(image.height * scale))), Image.LANCZOS)
    square = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    square.alpha_composite(image, ((size - image.width) // 2, (size - image.height) // 2))
    return square


os.makedirs("Art/UI/Markers", exist_ok=True)
for path in sorted(glob.glob("Art/AI/MapIcons/marker_*.png")):
    marker = fit(path, 256, 0.04)
    if marker:
        marker.save(os.path.join("Art/UI/Markers", os.path.basename(path)))
        print(os.path.basename(path))
