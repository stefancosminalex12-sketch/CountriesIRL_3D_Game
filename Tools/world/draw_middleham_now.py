"""
Top view of Middleham today (OpenStreetMap, (c) OpenStreetMap contributors, ODbL) over the real elevation
(AWS Terrain Tiles), about 3.2 x 3.2 km around the castle, with what the history books say about 1455 marked on it.
The frame shows the 1 x 1 km game area: at the towns' 1:3 scale it holds 3 x 3 km of the real place.
    python Tools/world/draw_middleham_now.py  ->  Docs/World/Middleham/middleham_now.png
Data from Art/MapData/Middleham (osm_middleham.json, elevation_middleham.npy).
"""
import json, math
import numpy as np
from PIL import Image, ImageDraw, ImageFont

LAT0, LON0, HALF = 54.2838, -1.8066, 1.6          # castle, half size in km
SIZE = 1600
osm = json.load(open("Art/MapData/Middleham/osm_middleham.json"))
elev = np.load("Art/MapData/Middleham/elevation_middleham.npy")

def xy(lat, lon):
    x = (lon - LON0) * 111.32 * math.cos(math.radians(LAT0)) / HALF
    y = (lat - LAT0) * 111.32 / HALF
    return ((x + 1) / 2 * SIZE, (1 - (y + 1) / 2) * SIZE)

# Hillshade (light from the north-west) tinted by height
e = np.array(Image.fromarray(elev).resize((SIZE, SIZE), Image.BICUBIC))
gy, gx = np.gradient(e, 3200 / SIZE)
shade = np.clip(0.6 + (-gx * 0.7 + gy * 0.7) * 1.8, 0.25, 1.15)
t = (e - e.min()) / (e.max() - e.min())
low, high = np.array([225, 222, 190]), np.array([170, 160, 125])
base = (low[None, None, :] * (1 - t[..., None]) + high[None, None, :] * t[..., None]) * shade[..., None]
img = Image.fromarray(np.clip(base, 0, 255).astype(np.uint8)).convert("RGBA")
d = ImageDraw.Draw(img, "RGBA")

# Contours every 10 m (from the same grid)
for level in range(100, 230, 10):
    mask = e >= level
    edge = mask ^ np.roll(mask, 1, 0) | mask ^ np.roll(mask, 1, 1)
    ys, xs = np.nonzero(edge)
    col = (120, 100, 70, 90) if level % 50 else (100, 80, 50, 150)
    for x, y in zip(xs[::2], ys[::2]):
        d.point((x, y), fill=col)

def poly(el):
    return [xy(p["lat"], p["lon"]) for p in el.get("geometry", [])]

order = []
for el in osm["elements"]:
    if el["type"] != "way" or "geometry" not in el:
        continue
    t_ = el.get("tags", {})
    pts = poly(el)
    if t_.get("landuse") in ("forest",) or t_.get("natural") == "wood":
        d.polygon(pts, fill=(90, 130, 70, 110))
    elif t_.get("natural") == "water":
        d.polygon(pts, fill=(110, 160, 200, 255))
    elif t_.get("landuse") in ("meadow", "grass", "village_green", "recreation_ground"):
        d.polygon(pts, fill=(170, 200, 120, 70))
for el in osm["elements"]:
    if el["type"] != "way" or "geometry" not in el:
        continue
    t_ = el.get("tags", {}); pts = poly(el)
    if t_.get("waterway") == "river":
        d.line(pts, fill=(80, 140, 200, 255), width=9)
    elif t_.get("waterway") in ("stream", "ditch"):
        d.line(pts, fill=(80, 140, 200, 220), width=3)
    elif t_.get("barrier") in ("wall", "hedge"):
        d.line(pts, fill=(110, 95, 70, 120), width=1)
for el in osm["elements"]:
    if el["type"] != "way" or "geometry" not in el:
        continue
    t_ = el.get("tags", {}); pts = poly(el)
    hw = t_.get("highway")
    if hw in ("primary", "secondary", "tertiary"):
        d.line(pts, fill=(150, 60, 40, 255), width=6)
    elif hw in ("unclassified", "residential", "service"):
        d.line(pts, fill=(160, 110, 80, 230), width=3)
    elif hw in ("track", "bridleway", "path", "footway"):
        d.line(pts, fill=(140, 110, 80, 150), width=1)
    if "building" in t_ and len(pts) > 2:
        d.polygon(pts, fill=(90, 80, 75, 230))
    if t_.get("historic") == "castle":
        d.polygon(pts, outline=(170, 30, 30, 255), fill=(200, 60, 50, 140), width=3)

try:
    font = ImageFont.truetype("C:/Windows/Fonts/georgia.ttf", 26); small = ImageFont.truetype("C:/Windows/Fonts/georgia.ttf", 20)
except OSError:
    font = small = ImageFont.load_default()

def label(lat, lon, text, colour=(40, 20, 10), f=None, dot=True):
    x, y = xy(lat, lon)
    if dot:
        d.ellipse([x - 6, y - 6, x + 6, y + 6], fill=(170, 30, 30, 255))
    d.text((x + 10, y - 12), text, fill=colour, font=f or small, stroke_width=3, stroke_fill=(245, 240, 220))

for el in osm["elements"]:
    t_ = el.get("tags", {})
    name = t_.get("name")
    if el["type"] == "node" and name:
        label(el["lat"], el["lon"], name)
    elif name and (t_.get("historic") or t_.get("amenity") == "place_of_worship"):
        g = el["geometry"]; lat = sum(p["lat"] for p in g) / len(g); lon = sum(p["lon"] for p in g) / len(g)
        label(lat, lon, name, f=font)

# The 1 x 1 km game area = 3 x 3 km of the real place (towns at 1:3), centred on the castle
a, b = xy(LAT0 + 1.5 / 111.32, LON0 - 1.5 / (111.32 * math.cos(math.radians(LAT0)))), xy(LAT0 - 1.5 / 111.32, LON0 + 1.5 / (111.32 * math.cos(math.radians(LAT0))))
d.rectangle([a, b], outline=(30, 30, 120, 255), width=4)
d.text((a[0] + 10, a[1] + 8), "Game area 1 x 1 km  (= 3 x 3 km real, towns at 1:3)", fill=(30, 30, 120), font=font, stroke_width=3, stroke_fill=(245, 240, 220))
# Scale bar: 500 m real
x0 = 40; y0 = SIZE - 50; px = 500 / 3200 * SIZE
d.line([(x0, y0), (x0 + px, y0)], fill=(0, 0, 0), width=5)
d.text((x0, y0 - 34), "500 m real  (= 167 m in game)", fill=(0, 0, 0), font=small, stroke_width=3, stroke_fill=(245, 240, 220))
d.text((SIZE - 150, 30), "N  ^", fill=(0, 0, 0), font=font, stroke_width=3, stroke_fill=(245, 240, 220))
d.text((40, 30), "Middleham today  (OpenStreetMap, real heights)", fill=(40, 20, 10), font=font, stroke_width=3, stroke_fill=(245, 240, 220))
img.convert("RGB").save("Docs/World/Middleham/middleham_now.png")
print("saved")
