"""
Draw the death screen's blood drips (Art/AI/Menu/death_blood_drips.png): an uneven band of blood along the top edge
with drips running straight down from it, each ending in a drop. Transparent everywhere else. The game slides it
down from above the screen so the drips seem to run. Colours match the splatter image (death_blood.png).
    python Tools/make_blood_drips.py [seed]
"""
import sys
import numpy as np
from PIL import Image, ImageDraw, ImageFilter

W, H, S = 1536, 1024, 2          # final size, supersampling for smooth edges
rng = np.random.default_rng(int(sys.argv[1]) if len(sys.argv) > 1 else 1455)


def smooth_noise(n, points, low, high):
    """A wavy line of n values between low and high"""
    knots = rng.uniform(low, high, points)
    return np.interp(np.linspace(0, points - 1, n), np.arange(points), knots)


mask = Image.new("L", (W * S, H * S), 0)
draw = ImageDraw.Draw(mask)

# The band along the top: thicker in places, where the drips start
band = smooth_noise(W, 34, 22, 62) + smooth_noise(W, 180, -6, 6)
draw.polygon([(0, 0)] + [(x * S, band[x] * S) for x in range(W)] + [(W * S, 0)], fill=255)

# Drips: most short, a few long (the longest about half way down), spread along the edge
xs = np.sort(rng.choice(np.arange(30, W - 30), size=30, replace=False))
for x in xs:
    long_one = rng.random() < 0.22
    length = rng.uniform(300, 520) if long_one else rng.uniform(60, 260)
    width = rng.uniform(16, 30) if long_one else rng.uniform(10, 22)
    top = band[x] - 4
    steps = int(length)
    wobble = smooth_noise(steps, 6, -3, 3)
    for i in range(steps):
        t = i / steps
        # Thins towards the end, swells again just before the drop
        w = width * (1.0 - 0.45 * t) * (1.0 + 0.25 * max(0.0, t - 0.85) / 0.15)
        cx, cy = (x + wobble[i]) * S, (top + i) * S
        draw.ellipse([cx - w * S / 2, cy - S, cx + w * S / 2, cy + S], fill=255)
    # The drop at the end
    r = width * 0.75
    cx, cy = (x + wobble[-1]) * S, (top + length) * S
    draw.ellipse([cx - r * S, cy - r * S * 1.1, cx + r * S, cy + r * S * 1.2], fill=255)

mask = mask.resize((W, H), Image.LANCZOS).filter(ImageFilter.GaussianBlur(0.8))
m = np.asarray(mask, dtype=np.float32) / 255.0

# Colour: the splatter's deep red, a little uneven, darker and browner towards the edges where it dries
grain = np.asarray(Image.fromarray((rng.random((H // 8, W // 8)) * 255).astype(np.uint8)).resize((W, H), Image.BICUBIC), dtype=np.float32) / 255.0
edge = np.clip(1.0 - np.asarray(mask.filter(ImageFilter.GaussianBlur(4)), dtype=np.float32) / 255.0 * 1.6, 0, 1)
red = 108 + 34 * (grain - 0.5) - 45 * edge
green = 2 + 10 * edge
blue = 4 + 6 * edge
rgba = np.dstack([np.clip(c, 0, 255) for c in (red, green, blue)] + [np.clip(m * 250, 0, 255)]).astype(np.uint8)
Image.fromarray(rgba, "RGBA").save("Art/AI/Menu/death_blood_drips.png")
print("drips drawn:", len(xs))
