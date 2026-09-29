"""
Draw the planning map of the game world: England in 1455 in the style of the Gough Map (parchment, green sea and
rivers, red roads, little buildings for towns), with shaded hills and a grid in GAME kilometres.
    python Tools/world/draw_plan_map.py [compression]      (default 12: 12 real km = 1 game km)
Reads Data/World/Places_England1455.csv and the data in Art/MapData (Natural Earth, OpenStreetMap rivers,
AWS Terrain Tiles; fetch with Tools/world/fetch_map_data.py).
Writes Docs/World/plan_map_england_1455.png (full size) and prints the world size.
"""
import csv
import json
import math
import os
import random
import sys

import numpy as np
from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont

Image.MAX_IMAGE_PIXELS = None
ROOT = "C:/Dev/CountriesIRL_3D_Game"
DATA = f"{ROOT}/Art/MapData"
COMPRESSION = float(sys.argv[1]) if len(sys.argv) > 1 else 12.0

# Projection: equirectangular around the middle of England, in real kilometres
LAT0, LON0 = 53.0, -1.9
KX = 111.32 * math.cos(math.radians(LAT0))
KY = 110.57
BOX = (-3.8, 50.5, 1.8, 55.83)       # lon/lat window drawn: England of the first release, nothing else
PX_PER_KM = 13.0                     # image scale (real km)
S = 2.25                             # size factor for symbols and text (designed at 4 px/km, kept when zooming in)
MARGIN = int(110 * S)
TOP = int(150 * S)

PARCH = (247, 239, 212)          # light yellowish paper
PARCH_DARK = (236, 224, 190)
SEA = (150, 178, 150)
RIVER = (78, 122, 106)
RIVER_TEXT = (44, 88, 76)
RIVER_NAV = (38, 88, 96)         # reach boats could use in 1455
CONTOUR = (150, 118, 80)
ROAD = (92, 58, 32)              # dark brown ink
INK = (58, 40, 26)
ROOF = (176, 52, 40)


def km(lon, lat):
    return (lon - LON0) * KX, (lat - LAT0) * KY


x_min, y_min = km(BOX[0], BOX[1])
x_max, y_max = km(BOX[2], BOX[3])
W = int((x_max - x_min) * PX_PER_KM) + 2 * MARGIN
H = int((y_max - y_min) * PX_PER_KM) + MARGIN + TOP + int(40 * S)


def px(lon, lat):
    x, y = km(lon, lat)
    return MARGIN + (x - x_min) * PX_PER_KM, TOP + (y_max - y) * PX_PER_KM


def unpx(X, Y):
    x = (X - MARGIN) / PX_PER_KM + x_min
    y = y_max - (Y - TOP) / PX_PER_KM
    return x / KX + LON0, y / KY + LAT0


def font(name, size):
    try:
        return ImageFont.truetype(f"{ROOT}/Art/Fonts/{name}.ttf", int(size * S))
    except OSError:
        return ImageFont.load_default()


COAST_KM = 1.5          # coast details smaller than this (real km, ~125 m in the game) are smoothed away
MIN_ISLAND_KM2 = 3.0    # smaller islands are left out


def game_coast(rings):
    """England's outline at game detail: simplified, rounded, without tiny islands (lon/lat rings)."""
    out = []
    for ring in rings:
        pts = [km(lon, lat) for lon, lat in ring[:-1]]
        area = abs(sum(pts[i][0] * pts[i - 1][1] - pts[i - 1][0] * pts[i][1] for i in range(len(pts)))) / 2
        if area < MIN_ISLAND_KM2 or len(pts) < 4:
            continue
        h = len(pts) // 2                                   # a closed ring is simplified in two halves
        simple = douglas_peucker(pts[:h + 1], COAST_KM)[:-1] + douglas_peucker(pts[h:] + [pts[0]], COAST_KM)[:-1]
        for _ in range(2):                                  # round the corners (Chaikin), ring closed
            simple = [q for a, b in zip(simple, simple[1:] + simple[:1])
                      for q in ((0.75 * a[0] + 0.25 * b[0], 0.75 * a[1] + 0.25 * b[1]), (0.25 * a[0] + 0.75 * b[0], 0.25 * a[1] + 0.75 * b[1]))]
        out.append([unkm(x, y) for x, y in simple])
    return out


def douglas_peucker(pts, tol):
    if len(pts) < 3:
        return pts
    a, b = pts[0], pts[-1]
    norm = math.hypot(b[0] - a[0], b[1] - a[1]) or 1e-12
    far, idx = 0.0, 0
    for i in range(1, len(pts) - 1):
        dd = abs((b[1] - a[1]) * pts[i][0] - (b[0] - a[0]) * pts[i][1] + b[0] * a[1] - b[1] * a[0]) / norm
        if dd > far:
            far, idx = dd, i
    if far <= tol:
        return [a, b]
    return douglas_peucker(pts[:idx + 1], tol)[:-1] + douglas_peucker(pts[idx:], tol)


def unkm(x, y):
    return x / KX + LON0, y / KY + LAT0


def geo_polys(geometry):
    if geometry["type"] == "Polygon":
        return [geometry["coordinates"]]
    if geometry["type"] == "MultiPolygon":
        return geometry["coordinates"]
    return []


def geo_lines(geometry):
    if not geometry:
        return []
    if geometry["type"] == "LineString":
        return [geometry["coordinates"]]
    if geometry["type"] == "MultiLineString":
        return geometry["coordinates"]
    return []


def in_box(lon, lat):
    return BOX[0] <= lon <= BOX[2] and BOX[1] <= lat <= BOX[3]


TERRAIN = f"{ROOT}/Data/World/Terrain_England1455"
# Height bands of the game terrain (game metres) and their paper tints, blended between
TERRAIN_TINTS = [(0, (247, 239, 212)), (8, (243, 233, 200)), (40, (233, 218, 178)), (110, (218, 198, 160)), (200, (200, 184, 160))]
CONTOUR_M = 50          # contour line every 50 m of game height


def terrain_layer(img, land):
    """The game terrain (Tools/world/build_terrain.py) on the map: height tints, shading from the game's own slopes
    (light from the north-west) and contour lines. Returns the new image, or the old one if there is no terrain yet."""
    if not os.path.exists(TERRAIN + ".png"):
        return img
    meta = json.load(open(TERRAIN + ".json"))
    h = np.asarray(Image.open(TERRAIN + ".png"), dtype=np.float32) / meta["value_per_game_m"]
    cell = meta["cell_game_m"]
    gy, gx = np.gradient(h, cell)                               # game slopes
    az, alt = math.radians(315), math.radians(45)
    slope = np.arctan(1.5 * np.hypot(gx, gy))                   # shading drawn 1.5x steep so gentle hills read
    aspect = np.arctan2(-gx, gy)
    hs = np.sin(alt) * np.cos(slope) + np.cos(alt) * np.sin(slope) * np.cos(az - aspect)
    shade = np.clip(1 + 0.6 * (hs - math.sin(alt)), 0.72, 1.06)
    # onto the map: the terrain box is the map's BOX, drawn inside the margins
    size = (int((x_max - x_min) * PX_PER_KM), int((y_max - y_min) * PX_PER_KM))
    hm = np.asarray(Image.fromarray(h).resize(size, Image.BICUBIC))
    sm = np.asarray(Image.fromarray(shade.astype(np.float32)).resize(size, Image.BICUBIC))
    stops = [t[0] for t in TERRAIN_TINTS]
    tint = np.stack([np.interp(hm, stops, [t[1][c] for t in TERRAIN_TINTS]) for c in range(3)], axis=-1)
    base = np.asarray(img, dtype=np.float32)[TOP:TOP + size[1], MARGIN:MARGIN + size[0]]
    grain = base / np.array(PARCH, dtype=np.float32)            # keep the paper grain drawn before
    out = np.clip(tint * grain * sm[..., None], 0, 255)
    band = np.floor(hm / CONTOUR_M)
    edge = np.zeros(hm.shape, bool)
    edge[:-1, :] |= band[:-1, :] != band[1:, :]
    edge[:, :-1] |= band[:, :-1] != band[:, 1:]
    out[edge] = out[edge] * 0.55 + np.array(CONTOUR, dtype=np.float32) * 0.45
    layer = img.copy()
    layer.paste(Image.fromarray(out.astype(np.uint8)), (MARGIN, TOP))
    return Image.composite(layer, img, land)


ICONS_DIR = f"{ROOT}/Art/AI/MapIcons"
# Which icon each type uses (the user's hand-painted icons in Art/AI/MapIcons/map_*.png)
ICON_FOR = {"City": "map_city", "Town": "map_town", "Village": "map_village", "Cathedral": "map_cathedral",
            "Abbey": "map_abbey", "Battle": "map_battle", "Landmark": "map_landmark", "Nature": "map_nature"}
# Icon size (map pixels at 4 px/km, times S) per type and importance
ICON_SIZE = {("City", 3): 44, ("City", 2): 34, ("City", 1): 30, ("Town", 3): 40, ("Town", 2): 26, ("Town", 1): 17,
             ("Village", 1): 7, ("Castle", 3): 22, ("Castle", 2): 18, ("Castle", 1): 10, ("Cathedral", 3): 24,
             ("Cathedral", 2): 20, ("Cathedral", 1): 16, ("Abbey", 3): 16, ("Abbey", 2): 13, ("Abbey", 1): 8,
             ("Battle", 3): 22, ("Battle", 2): 16, ("Battle", 1): 14, ("Landmark", 3): 22, ("Landmark", 2): 18,
             ("Landmark", 1): 14, ("Nature", 3): 18, ("Nature", 2): 16, ("Nature", 1): 13}


def icon_name(kind, imp):
    if kind == "Castle":
        return "map_castle_major" if imp >= 2 else "map_castle_minor"
    if kind == "Town" and imp >= 3:
        return "map_city"
    return ICON_FOR.get(kind, "")


def load_icons():
    """Hand-painted map icons (Art/AI/MapIcons/map_*.png), trimmed; missing ones fall back to drawn symbols."""
    icons = {}
    if os.path.isdir(ICONS_DIR):
        for f in os.listdir(ICONS_DIR):
            if f.startswith("map_") and f.endswith(".png"):
                im = Image.open(os.path.join(ICONS_DIR, f)).convert("RGBA")
                bbox = im.getchannel("A").point(lambda a: 255 if a > 8 else 0).getbbox()
                icons[f[:-4]] = im.crop(bbox) if bbox else im
    return icons


def draw_symbol(img, d, icons, kind, imp, X, Y):
    """Draws a place's symbol and returns its radius (for label placement)."""
    size = ICON_SIZE.get((kind, imp), 12) * S
    icon = icons.get(icon_name(kind, imp))
    if icon is not None:
        scale = size / max(icon.size)
        ic = icon.resize((max(1, int(icon.width * scale)), max(1, int(icon.height * scale))), Image.LANCZOS)
        img.paste(ic, (int(X - ic.width / 2), int(Y - ic.height / 2)), ic)
        return size / 2
    s = size / 2
    if kind in ("City", "Town"):
        d.rectangle([X - s, Y - s * 0.4, X + s, Y + s * 0.9], fill=(240, 232, 214), outline=INK, width=2)
        d.polygon([(X - s * 1.15, Y - s * 0.4), (X, Y - s * 1.4), (X + s * 1.15, Y - s * 0.4)], fill=ROOF, outline=INK)
        if kind == "City":
            d.rectangle([X + s * 0.35, Y - s * 1.9, X + s * 0.8, Y - s * 0.4], fill=(240, 232, 214), outline=INK, width=2)
    elif kind == "Castle":
        d.rectangle([X - s, Y - s, X + s, Y + s], fill=(118, 108, 98), outline=INK, width=max(1, int(S)))
        if imp >= 2:
            for k in (-1, 0, 1):
                d.rectangle([X + k * s * 0.7 - s * 0.25, Y - s * 1.45, X + k * s * 0.7 + s * 0.25, Y - s], fill=(118, 108, 98), outline=INK)
    elif kind in ("Abbey", "Cathedral"):
        col = INK if kind == "Cathedral" or imp >= 2 else (95, 75, 60)
        d.line([(X, Y - s), (X, Y + s)], fill=col, width=max(2, int(s / 3)))
        d.line([(X - s * 0.7, Y - s * 0.35), (X + s * 0.7, Y - s * 0.35)], fill=col, width=max(2, int(s / 3)))
    elif kind == "Landmark":
        for k in (-1, 0, 1):
            d.rectangle([X + k * s * 0.8 - s * 0.25, Y - s * 0.9, X + k * s * 0.8 + s * 0.25, Y + s * 0.5], fill=(150, 140, 125), outline=INK)
    elif kind == "Nature":
        d.polygon([(X - s, Y + s * 0.6), (X, Y - s * 0.9), (X + s, Y + s * 0.6)], fill=(120, 150, 100), outline=INK)
    elif kind == "Battle":
        d.line([(X - s, Y - s), (X + s, Y + s)], fill=(150, 20, 20), width=max(2, int(1.6 * S)))
        d.line([(X - s, Y + s), (X + s, Y - s)], fill=(150, 20, 20), width=max(2, int(1.6 * S)))
    elif kind == "Village":
        d.ellipse([X - s, Y - s, X + s, Y + s], fill=INK)
    return s


PX_PER_GAME_M = PX_PER_KM * COMPRESSION / 1000          # map pixels per metre of game world (~0.16: 1 px = ~6.4 m)


def river_points(r):
    """Pixel points of a river and, for each, how far along it is (0 = source, 1 = mouth)."""
    pts = [px(lon, lat) for lon, lat in r["line"]]
    run = [0.0]
    for a, b in zip(pts, pts[1:]):
        run.append(run[-1] + math.hypot(b[0] - a[0], b[1] - a[1]))
    return pts, [v / (run[-1] or 1) for v in run]


def river_width(r, t):
    """True width in the game (1:3 of real), widening towards the mouth; at least 2 px so it shows."""
    w0, w1 = r["width_game_m"]
    return max(2.0, (w0 + (w1 - w0) * math.sqrt(t)) * PX_PER_GAME_M)


def draw_rivers(d, rivers):
    """Small rivers first so the big ones sit on top; the reach boats could use in 1455 is a darker blue."""
    for r in sorted(rivers, key=lambda r: -r["tier"]):
        pts, ts = river_points(r)
        nav = r["navigable_from"]
        for (a, b), t in zip(zip(pts, pts[1:]), ts[1:]):
            w = river_width(r, t)
            colour = RIVER_NAV if nav is not None and t > nav else RIVER
            d.line([a, b], fill=colour, width=max(2, round(w)))
            d.ellipse([b[0] - w / 2, b[1] - w / 2, b[0] + w / 2, b[1] + w / 2], fill=colour)


def label_rivers(img, rivers, obstacles, fnt, land):
    """River names written along the river (Gough-style), only where they cover nothing else."""
    for r in sorted(rivers, key=lambda r: r["tier"]):
        pts, ts = river_points(r)
        text = r["name"]
        box = fnt.getbbox(text)
        tw, th = box[2] - box[0], box[3] - box[1]
        for want in (0.55, 0.4, 0.7, 0.3, 0.8, 0.2):
            i = min(range(len(ts)), key=lambda k: abs(ts[k] - want))
            a, b = pts[max(0, i - 3)], pts[min(len(pts) - 1, i + 3)]
            ang = math.degrees(math.atan2(-(b[1] - a[1]), b[0] - a[0]))
            if ang > 90:
                ang -= 180
            elif ang < -90:
                ang += 180
            if abs(ang) > 70:                     # nearly vertical river: keep the text readable
                continue
            off = river_width(r, ts[i]) / 2 + th * 0.8
            nx, ny = -math.sin(math.radians(ang)) * off, -math.cos(math.radians(ang)) * off
            cx, cy = pts[i][0] + nx, pts[i][1] + ny
            if not (0 <= cx < img.width and 0 <= cy < img.height) or land.getpixel((int(cx), int(cy))) == 0:
                continue                          # keep names on England
            layer = Image.new("RGBA", (tw + 8, th + 8), (0, 0, 0, 0))
            ImageDraw.Draw(layer).text((4 - box[0], 4 - box[1]), text, font=fnt, fill=RIVER_TEXT + (255,),
                                       stroke_width=max(1, int(S)), stroke_fill=PARCH + (255,))
            layer = layer.rotate(ang, expand=True, resample=Image.BICUBIC)
            x0, y0 = int(cx - layer.width / 2), int(cy - layer.height / 2)
            rect = (x0 + 4, y0 + 4, x0 + layer.width - 4, y0 + layer.height - 4)
            if any(rect[0] < o[2] and rect[2] > o[0] and rect[1] < o[3] and rect[3] > o[1] for o in obstacles):
                continue
            img.paste(layer, (x0, y0), layer)
            obstacles.append(rect)
            break


CUT = json.load(open(f"{ROOT}/Data/World/World_Release1.json", encoding="utf-8"))["cut_areas"]


def release_decisions():
    """Id -> decision from Data/World/Places_Release1.csv ("cut", "village", "part_of:<id>")."""
    path = f"{ROOT}/Data/World/Places_Release1.csv"
    if not os.path.exists(path):
        return {}
    return {r["Id"]: r["Decision"] for r in csv.DictReader(open(path, encoding="utf-8"))}


CASTLE_IMPORTANCE = {"large": "3", "medium": "2", "small": "1"}


def release_places(all_places, units):
    """The places of the first release: inside playable England, with Places_Release1.csv applied (cut / village /
    part_of) and only the castles of Castles_Release1.csv, their size as Importance and their type as CastleType."""
    wales = [poly[0] for f in units["features"] if f["properties"].get("NAME") == "Wales" for poly in geo_polys(f["geometry"])]
    inside = lambda r: not is_cut(float(r["Lon"]), float(r["Lat"])) and not any(in_poly(float(r["Lon"]), float(r["Lat"]), w) for w in wales)
    places = {i: r for i, r in all_places.items() if inside(r)}
    for pid, decision in release_decisions().items():
        if pid not in places:
            continue
        if decision == "village":
            places[pid] = dict(places[pid], Type="Village", Importance="1")
        else:                                                  # "cut", or "part_of:<site>" (drawn with that site)
            del places[pid]
    castles_path = f"{ROOT}/Data/World/Castles_Release1.csv"
    if os.path.exists(castles_path):
        castles = {r["Id"]: r for r in csv.DictReader(open(castles_path, encoding="utf-8"))}
        for pid in [i for i, r in places.items() if r["Type"] == "Castle"]:
            c = castles.get(pid)
            if c is None:
                del places[pid]
            else:
                places[pid] = dict(places[pid], Importance=CASTLE_IMPORTANCE[c["Size"]], CastleType=c["Type"],
                                   Note=f"{c['Size']} {c['Type']} castle; {c['Holder1455']}; {c['Why']}")
    return places


def in_poly(lon, lat, poly):
    inside = False
    for (x1, y1), (x2, y2) in zip(poly, poly[1:] + poly[:1]):
        if (y1 > lat) != (y2 > lat) and lon < x1 + (lat - y1) * (x2 - x1) / (y2 - y1):
            inside = not inside
    return inside


def is_cut(lon, lat):
    """True if the point is in an area left out of the first release (Data/World/World_Release1.json)."""
    return any(in_poly(lon, lat, a["polygon"]) for a in CUT)


def road_in_release(line):
    """The part of a road before it enters a cut area, ending exactly at the edge."""
    out = [line[0]]
    for a, b in zip(line, line[1:]):
        if is_cut(*b):
            lo, hi = 0.0, 1.0
            for _ in range(20):                                # find where the road crosses the edge
                mid = (lo + hi) / 2
                if is_cut(a[0] + (b[0] - a[0]) * mid, a[1] + (b[1] - a[1]) * mid):
                    hi = mid
                else:
                    lo = mid
            out.append([a[0] + (b[0] - a[0]) * lo, a[1] + (b[1] - a[1]) * lo])
            break
        out.append(b)
    return out


def draw_rulers(d, box, game_km_px):
    """Rulers in game km along the bottom and the left of the map: a tick every km, a number every 5 km,
    zero at England's west and south edges, the full length at the far end."""
    x0, y0, x1, y1 = box
    f, fb = font("EBGaramond-Regular", 20), font("EBGaramond-SemiBold", 24)
    width_km, height_km = (x1 - x0) / game_km_px, (y1 - y0) / game_km_px
    yb = H - MARGIN + int(8 * S)                               # bottom ruler, under the map frame
    d.line([(x0, yb), (x1, yb)], fill=INK, width=3)
    for k in range(int(width_km) + 1):
        X = x0 + k * game_km_px
        big = k % 5 == 0
        d.line([(X, yb), (X, yb + (22 if big else 11))], fill=INK, width=3 if big else 2)
        if big:
            d.text((X, yb + 26), f"{k}", font=f, fill=INK, anchor="mt")
    d.line([(x1, yb - 8), (x1, yb + 22)], fill=INK, width=4)
    d.text((x1 + 10, yb + 2), f"{width_km:.1f} km", font=fb, fill=INK, anchor="lt")
    xl = MARGIN - int(8 * S)                                   # left ruler, beside the map frame
    d.line([(xl, y0), (xl, y1)], fill=INK, width=3)
    for k in range(int(height_km) + 1):
        Y = y1 - k * game_km_px
        big = k % 5 == 0
        d.line([(xl, Y), (xl - (22 if big else 11), Y)], fill=INK, width=3 if big else 2)
        if big:
            d.text((xl - 28, Y), f"{k}", font=f, fill=INK, anchor="rm")
    d.line([(xl - 22, y0), (xl + 8, y0)], fill=INK, width=4)
    d.text((xl - 28, y0), f"{height_km:.1f} km", font=fb, fill=INK, anchor="rm")
    d.text((x0, yb + 52), "game kilometres (1 game km = 12 real km)", font=font("EBGaramond-Italic", 20), fill=INK, anchor="lt")


def main():
    rnd = random.Random(3)

    # Sea with soft painted blotches
    img = Image.new("RGB", (W, H), SEA)
    blot = Image.new("L", (W // 40, H // 40))
    blot.putdata([rnd.randint(0, 45) for _ in range(blot.width * blot.height)])
    blot = blot.resize((W, H), Image.BICUBIC).filter(ImageFilter.GaussianBlur(30))
    img = Image.composite(Image.new("RGB", (W, H), (124, 158, 132)), img, blot)

    # Land: only the England of the first release. Wales, Scotland and the cut areas are not drawn at all.
    sea = img.copy()
    land = Image.new("L", (W, H), 0)
    ld = ImageDraw.Draw(land)
    d = ImageDraw.Draw(img)
    units = json.load(open(f"{DATA}/ne_10m_admin_0_map_units.geojson", encoding="utf-8"))
    for f in units["features"]:
        name = f["properties"].get("NAME")
        if name != "England":
            continue
        coast = game_coast([poly[0] for poly in geo_polys(f["geometry"])])
        for ring in coast:
            d.polygon([px(lon, lat) for lon, lat in ring], fill=PARCH)
            ld.polygon([px(lon, lat) for lon, lat in ring], fill=255)
    cut = Image.new("L", (W, H), 0)                            # areas left out of the first release
    cd = ImageDraw.Draw(cut)
    for a in CUT:
        cd.polygon([px(lon, lat) for lon, lat in a["polygon"]], fill=255)
    land = ImageChops.subtract(land, cut)
    img = Image.composite(img, sea, land)
    d = ImageDraw.Draw(img)
    england_area = land.histogram()[255] / PX_PER_KM ** 2      # playable England, real km2
    england_area = land.histogram()[255] / PX_PER_KM ** 2      # playable England, real km2

    # Parchment grain on England
    grain = Image.new("L", (W // 14, H // 14))
    grain.putdata([rnd.randint(0, 255) for _ in range(grain.width * grain.height)])
    grain = grain.resize((W, H), Image.BICUBIC).filter(ImageFilter.GaussianBlur(8)).point(lambda v: 60 if v > 150 else 0)
    img = Image.composite(Image.new("RGB", (W, H), PARCH_DARK), img, ImageChops.multiply(grain, land))

    # Terrain: the game's generalised heights, not the real ones
    img = terrain_layer(img, land)

    d = ImageDraw.Draw(img)
    # Outline: England's coast and land border (not along cut areas), and the edge of each cut area
    outline_w = max(2, int(1.5 * S))
    for ring in coast:
        ring = ring + [ring[0]]
        for a, b in zip(ring, ring[1:]):
            if not is_cut(*a) and not is_cut(*b):
                d.line([px(*a), px(*b)], fill=INK, width=outline_w)
    for a in CUT:
        d.line([px(lon, lat) for lon, lat in a["polygon"][:a["border_points"]]], fill=INK, width=outline_w, joint="curve")

    # Rivers: the game's simplified, connected network (Tools/world/build_rivers.py), wider towards the mouth.
    # Drawn on their own layer and kept to England, so the stretches in Wales or cut areas do not show.
    rivers = [r for r in json.load(open(f"{ROOT}/Data/World/Rivers_England1455.json", encoding="utf-8"))["rivers"]
              if not is_cut(*r["line"][-1])]                   # rivers reaching the sea in a cut area are left out
    layer = img.copy()
    draw_rivers(ImageDraw.Draw(layer), [r for r in rivers if not r.get("border")])
    img = Image.composite(layer, img, land.filter(ImageFilter.MaxFilter(7)))
    layer = img.copy()                                         # rivers forming the border (Tweed) show on both banks
    draw_rivers(ImageDraw.Draw(layer), [r for r in rivers if r.get("border")])
    img = Image.composite(layer, img, land.filter(ImageFilter.MaxFilter(25)))
    d = ImageDraw.Draw(img)

    all_places = {row["Id"]: row for row in csv.DictReader(open(f"{ROOT}/Data/World/Places_England1455.csv", encoding="utf-8"))}
    places = {i: r for i, r in release_places(all_places, units).items() if r.get("CastleType") != "town"}  # town castles: part of the town
    P = lambda i: px(float(all_places[i]["Lon"]), float(all_places[i]["Lat"]))

    # Main roads of 1455 (Tools/world/build_roads.py). Drawn 3x their real width so they show: at this scale
    # 1 px is ~6.4 m in the game (roads keep their real width), so a true 8 m road would be barely one pixel.
    roads = json.load(open(f"{ROOT}/Data/World/Roads_England1455.json", encoding="utf-8"))["roads"]
    px_per_game_m = PX_PER_KM * COMPRESSION / 1000
    for road in roads:
        w = max(2, round(road["width_m"] * 3 * px_per_game_m))
        d.line([px(*p) for p in road_in_release(road["line"])], fill=ROAD, width=w, joint="curve")

    # Grid in GAME kilometres, measured from England's west and south edges (the rulers' zero)
    step_game = 5
    step_real = step_game * COMPRESSION
    game_km_px = PX_PER_KM * COMPRESSION
    land_box = land.getbbox()                                  # playable England, in map pixels
    ox, oy = land_box[0], land_box[3]
    for k in range(-5, 40):
        X = ox + k * step_game * game_km_px
        if MARGIN <= X <= W - MARGIN:
            d.line([(X, TOP), (X, H - MARGIN)], fill=(90, 70, 50), width=1)
        Y = oy - k * step_game * game_km_px
        if TOP <= Y <= H - MARGIN:
            d.line([(MARGIN, Y), (W - MARGIN, Y)], fill=(90, 70, 50), width=1)
    size_game = ((land_box[2] - land_box[0]) / game_km_px, (land_box[3] - land_box[1]) / game_km_px)

    # ---- Places: symbols first, then labels placed so they do not overlap ----
    f_city = font("Cinzel-Bold", 30)
    f_town = font("Cinzel-SemiBold", 21)
    f_minor = font("EBGaramond-SemiBold", 17)
    f_small = font("EBGaramond-Italic", 15)
    f_battle = font("EBGaramond-Italic", 17)
    f_area = font("EBGaramond-Italic", 30)
    f_area_big = font("EBGaramond-Italic", 40)

    icons = load_icons()
    obstacles = []          # boxes labels must not cover: symbols and labels already placed
    labels = []             # (priority, text, font, colour, X, Y, radius, must_show, centred)

    order = {"Village": 0, "Abbey": 1, "Nature": 2, "Landmark": 3, "Castle": 4, "Cathedral": 5, "Battle": 6, "Town": 7, "City": 8}
    for row in sorted(places.values(), key=lambda r: (order.get(r["Type"], 0), int(r["Importance"]))):
        kind, imp = row["Type"], int(row["Importance"])
        X, Y = px(float(row["Lon"]), float(row["Lat"]))
        name = row["Name"]
        if kind == "Area":
            big = imp >= 2
            labels.append((1 + imp, name.upper() if big else name, f_area_big if big else f_area, (110, 84, 58), X, Y, 0, False, True))
            continue
        r = draw_symbol(img, d, icons, kind, imp, X, Y)
        if kind in ("City", "Town", "Cathedral", "Battle") or (kind == "Castle" and imp >= 2) or imp >= 2:
            obstacles.append((X - r, Y - r, X + r, Y + r))
        if kind in ("City", "Town"):
            fnt = f_city if imp == 3 else (f_town if imp == 2 else f_minor)
            labels.append(({3: 100, 2: 80, 1: 50}[imp] + (5 if kind == "City" else 0), name, fnt, INK, X, Y, r, imp >= 2, False))
        elif kind == "Castle" and imp >= 2:
            labels.append((40 + imp, name.replace(" Castle", ""), f_minor, (70, 60, 55), X, Y, r, False, False))
        elif kind == "Cathedral" and imp >= 2:
            labels.append((45 + imp, name, f_small, (70, 60, 55), X, Y, r, False, False))
        elif kind == "Abbey" and imp >= 2:
            labels.append((30, name, f_small, (70, 60, 55), X, Y, r, False, False))
        elif kind == "Landmark":
            labels.append((60 + imp, name, f_battle if imp >= 2 else f_small, (60, 50, 90), X, Y, r, imp >= 3, False))
        elif kind == "Nature":
            labels.append((35 + imp, name, f_small, (40, 80, 50), X, Y, r, False, False))
        elif kind == "Battle":
            labels.append((70 + imp * 5, name, f_battle, (130, 20, 20), X, Y, r, imp >= 2, False))
        elif kind == "Village":
            labels.append((20, name, f_small, (80, 64, 48), X, Y, r, False, False))

    pad = 2 * S
    for prio, text, fnt, colour, X, Y, r, must, centred in sorted(labels, key=lambda l: -l[0]):
        box = d.textbbox((0, 0), text, font=fnt)
        tw, th = box[2] - box[0], box[3] - box[1]
        g = r + 3 * S
        if centred:
            spots = [(X - tw / 2, Y - th / 2 + k * th * 0.9) for k in (0, 1, -1, 2, -2)]
        else:
            spots = [(X + g, Y - th / 2), (X - g - tw, Y - th / 2), (X - tw / 2, Y - g - th), (X - tw / 2, Y + g),
                     (X + g, Y - g - th * 0.6), (X + g, Y + g * 0.4), (X - g - tw, Y + g * 0.4), (X - g - tw, Y - g - th * 0.6)]
        chosen = None
        for sx, sy in spots:
            rect = (sx - pad, sy - pad, sx + tw + pad, sy + th + pad)
            if not any(rect[0] < o[2] and rect[2] > o[0] and rect[1] < o[3] and rect[3] > o[1] for o in obstacles):
                chosen = (sx, sy, rect)
                break
        if not chosen:
            if not must:
                continue            # no room: a minor label is left out rather than drawn over another
            def covered(s):         # how much of other labels/symbols this spot would cover
                rx0, ry0, rx1, ry1 = s[0] - pad, s[1] - pad, s[0] + tw + pad, s[1] + th + pad
                return sum(max(0, min(rx1, o[2]) - max(rx0, o[0])) * max(0, min(ry1, o[3]) - max(ry0, o[1])) for o in obstacles)
            sx, sy = min(spots, key=covered)
            chosen = (sx, sy, (sx - pad, sy - pad, sx + tw + pad, sy + th + pad))
        sx, sy, rect = chosen
        obstacles.append(rect)
        d.text((sx - box[0], sy - box[1]), text, font=fnt, fill=colour, stroke_width=max(1, int(1.2 * S)), stroke_fill=PARCH)

    label_rivers(img, rivers, obstacles, font("EBGaramond-Italic", 19), land)
    d = ImageDraw.Draw(img)

    # Title, scale, legend
    area_game = england_area / COMPRESSION ** 2
    counts = {}
    for r in places.values():
        counts[r["Type"]] = counts.get(r["Type"], 0) + 1
    d.text((MARGIN, int(30 * S)), "Crowns & Commoners - England, 1455", font=font("Cinzel-Bold", 56), fill=INK)
    d.multiline_text((MARGIN, int(92 * S)), f"Planning map. Compression 1:{COMPRESSION:g} ({COMPRESSION:g} real km = 1 game km). "
                                  f"Playable England {england_area:,.0f} km2 real -> about {area_game:,.0f} km2 in game.\n"
                                  f"Playable England is {size_game[0]:.0f} km wide and {size_game[1]:.0f} km tall in the game (rulers at the edges). "
                                  f"Grid squares = {step_game} x {step_game} game km ({step_real:g} real km), from England's west and south edges.",
           font=font("EBGaramond-Regular", 24), fill=INK, spacing=int(3 * S))
    legend = (f"Towns & cities {counts.get('Town', 0) + counts.get('City', 0)}  /  castles {counts.get('Castle', 0)}  /  "
              f"abbeys, priories & friaries {counts.get('Abbey', 0)}  /  cathedrals {counts.get('Cathedral', 0)}  /  "
              f"landmarks {counts.get('Landmark', 0)}  /  nature {counts.get('Nature', 0)}  /  battles {counts.get('Battle', 0)}  /  "
              f"villages {counts.get('Village', 0)}.\n"
              f"Brown lines: the {len(roads)} main roads (thicker: the great Roman-built roads). Rivers: the {len(rivers)} main rivers at their game width, darker where boats went in 1455.\n"
              f"Not shown: Devon & Cornwall (later update), Wales and Scotland (DLC). Terrain: the game's own heights (tints: lowland, hills, uplands, mountains; contours every 50 m of game height). "
              f"Data: Natural Earth, OpenStreetMap contributors, Wikidata, AWS Terrain Tiles.")
    d.rectangle([0, H - MARGIN + 1, W, H], fill=SEA)            # clean strip under the map frame for the legend
    d.multiline_text((MARGIN, H - MARGIN + int(56 * S)), legend, font=font("EBGaramond-Italic", 18), fill=INK, spacing=int(4 * S))
    draw_rulers(d, land_box, game_km_px)

    os.makedirs(f"{ROOT}/Docs/World", exist_ok=True)
    out = f"{ROOT}/Docs/World/plan_map_england_1455.png"
    # Save next to it, then swap in (an image viewer holding the old file can make a direct save fail)
    tmp = out.replace(".png", ".tmp.png")
    img.save(tmp, optimize=True)
    for attempt in range(10):
        try:
            os.replace(tmp, out)
            break
        except OSError:
            import time
            time.sleep(1)
    print(f"Playable England real area {england_area:,.0f} km2; at 1:{COMPRESSION:g} -> {area_game:,.0f} km2 in game; image {W}x{H}; {counts}")


if __name__ == "__main__":
    main()
