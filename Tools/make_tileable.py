"""
Make the AI textures tile without visible seams, for the game: Art/AI/Textures/tex_*.png -> Art/Textures/tex_*.png.
Where an edge doesn't match the opposite edge (measured the same way as the seam check), a band of the texture's own
far side is cross-faded into the near side, so the last column leads into the first. The band is cut off and the
texture scaled back to its size. Edges that already match are left alone. Originals stay untouched.
    python Tools/make_tileable.py
"""
import glob
import os
import numpy as np
from PIL import Image

SOURCE, OUT = "Art/AI/Textures", "Art/Textures"
LIMIT = 1.8      # edge difference compared with the inside of the texture: above this, the seam shows
BAND = 0.06      # share of the size cross-faded


def seam(a, axis):
    grey = a.mean(axis=2)
    inside = (np.abs(np.diff(grey, axis=1)).mean() + np.abs(np.diff(grey, axis=0)).mean()) / 2
    edge = np.abs(grey[:, 0] - grey[:, -1]).mean() if axis == 1 else np.abs(grey[0, :] - grey[-1, :]).mean()
    return edge / inside


def crossfade(a, axis):
    """Blends the far band into the near band along one axis, and drops the far band"""
    a = np.moveaxis(a, axis, 0)
    n = a.shape[0]
    b = int(n * BAND)
    t = np.linspace(0.0, 1.0, b)[:, None, None]
    out = a[: n - b].copy()
    out[:b] = a[n - b:] * (1 - t) + a[:b] * t
    return np.moveaxis(out, 0, axis)


os.makedirs(OUT, exist_ok=True)
for path in sorted(glob.glob(os.path.join(SOURCE, "tex_*.png"))):
    image = Image.open(path).convert("RGB")
    a = np.asarray(image, dtype=np.float32)
    fixed = []
    for axis, name in ((1, "left/right"), (0, "top/bottom")):
        if seam(a, axis) > LIMIT:
            a = crossfade(a, axis)
            fixed.append(name)
    result = Image.fromarray(np.clip(a, 0, 255).astype(np.uint8)).resize(image.size, Image.LANCZOS)
    result.save(os.path.join(OUT, os.path.basename(path)))
    after = np.asarray(result, dtype=np.float32)
    print("%-26s %-24s now %.2f / %.2f" % (os.path.basename(path), ", ".join(fixed) or "fine as it was", seam(after, 1), seam(after, 0)))
