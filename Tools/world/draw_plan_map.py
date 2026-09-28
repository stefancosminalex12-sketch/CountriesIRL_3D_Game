"""
Draw the planning map of the game world: England in 1455 in the style of the Gough Map (parchment, green sea and
rivers, red roads, little buildings for towns), with a grid in GAME kilometres to show how big the world is.
    python Tools/world/draw_plan_map.py [compression]      (default 12: 12 real km = 1 game km)
Reads Data/World/Places_England1455.csv and the Natural Earth data in Art/MapData (public domain).
Writes Docs/World/plan_map_england_1455.png and prints the world size.
"""
import csv
import json
import math
import os
import random
import sys

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = "C:/Dev/CountriesIRL_3D_Game"
COMPRESSION = float(sys.argv[1]) if len(sys.argv) > 1 else 12.0

# Projection: equirectangular around the middle of England, in real kilometres
LAT0, LON0 = 53.0, -1.9
KX = 111.32 * math.cos(math.radians(LAT0))
KY = 110.57
BOX = (-5.9, 49.8, 2.0, 55.95)       # lon/lat window drawn
PX_PER_KM = 4.0                      # image scale (real km)
MARGIN = 170

PARCH = (236, 222, 188)
PARCH_DARK = (214, 194, 150)
SEA = (150, 178, 150)
RIVER = (86, 128, 110)
ROAD = (170, 48, 34)
INK = (58, 40, 26)
ROOF = (176, 52, 40)
FADED = (196, 192, 176)


def km(lon, lat):
    return (lon - LON0) * KX, (lat - LAT0) * KY


x_min, y_min = km(BOX[0], BOX[1])
x_max, y_max = km(BOX[2], BOX[3])
W = int((x_max - x_min) * PX_PER_KM) + 2 * MARGIN
H = int((y_max - y_min) * PX_PER_KM) + 2 * MARGIN + 160


def px(lon, lat):
    x, y = km(lon, lat)
    return MARGIN + (x - x_min) * PX_PER_KM, MARGIN + 120 + (y_max - y) * PX_PER_KM


def font(name, size):
    try:
        return ImageFont.truetype(f"{ROOT}/Art/Fonts/{name}.ttf", size)
    except OSError:
        return ImageFont.load_default()


def polygons(geometry):
    if geometry["type"] == "Polygon":
        return [geometry["coordinates"]]
    if geometry["type"] == "MultiPolygon":
        return geometry["coordinates"]
    return []


def lines(geometry):
    if geometry["type"] == "LineString":
        return [geometry["coordinates"]]
    if geometry["type"] == "MultiLineString":
        return geometry["coordinates"]
    return []


def in_box(lon, lat):
    return BOX[0] <= lon <= BOX[2] and BOX[1] <= lat <= BOX[3]


def main():
    img = Image.new("RGB", (W, H), SEA)
    # Painted sea: soft blotches
    rnd = random.Random(3)
    noise = Image.new("L", (W // 24, H // 24))
    noise.putdata([rnd.randint(0, 40) for _ in range(noise.width * noise.height)])
    noise = noise.resize((W, H), Image.BICUBIC).filter(ImageFilter.GaussianBlur(18))
    img = Image.composite(Image.new("RGB", (W, H), (126, 160, 134)), img, noise)
    d = ImageDraw.Draw(img)

    # Land: England bright, Wales / Scotland faded (future DLC regions)
    units = json.load(open(f"{ROOT}/Art/MapData/ne_10m_admin_0_map_units.geojson", encoding="utf-8"))
    england_area = 0.0
    for f in units["features"]:
        name = f["properties"].get("NAME")
        if f["properties"].get("ADMIN") not in ("United Kingdom", "France", "Ireland", "Isle of Man"):
            continue
        fill = PARCH if name == "England" else FADED
        for poly in polygons(f["geometry"]):
            ring = [px(lon, lat) for lon, lat in poly[0]]
            d.polygon(ring, fill=fill, outline=INK)
            if name == "England":
                pts = [km(lon, lat) for lon, lat in poly[0]]
                england_area += abs(sum(pts[i][0] * pts[i - 1][1] - pts[i - 1][0] * pts[i][1] for i in range(len(pts)))) / 2
    # Parchment grain on land (darker blotches where the noise is high)
    grain = Image.new("L", (W // 10, H // 10))
    grain.putdata([rnd.randint(0, 255) for _ in range(grain.width * grain.height)])
    grain = grain.resize((W, H), Image.BICUBIC).filter(ImageFilter.GaussianBlur(6)).point(lambda v: 70 if v > 150 else 0)
    land = img.convert("L").point(lambda v: 255 if v > 210 else 0)
    img = Image.composite(Image.new("RGB", (W, H), PARCH_DARK), img, Image.composite(grain, Image.new("L", (W, H), 0), land))
    d = ImageDraw.Draw(img)

    # Rivers (green, like the Gough Map)
    for fname in ("ne_10m_rivers_lake_centerlines", "ne_10m_rivers_europe"):
        rivers = json.load(open(f"{ROOT}/Art/MapData/{fname}.geojson", encoding="utf-8"))
        for f in rivers["features"]:
            for line in lines(f["geometry"]):
                if not any(in_box(lon, lat) for lon, lat in line):
                    continue
                d.line([px(lon, lat) for lon, lat in line], fill=RIVER, width=4, joint="curve")

    places = {row["Id"]: row for row in csv.DictReader(open(f"{ROOT}/Data/World/Places_England1455.csv", encoding="utf-8"))}
    P = lambda i: px(float(places[i]["Lon"]), float(places[i]["Lat"]))

    # Main roads of the period (after the Gough Map's red routes)
    roads = [
        ["london", "ware", "huntingdon", "stamford", "grantham", "newark", "doncaster", "pontefract", "tadcaster", "york"],
        ["york", "ripon", "richmond", "durham", "newcastle", "alnwick", "berwick"],
        ["ripon", "middleham", "richmond"],
        ["london", "st_albans", "northampton", "coventry", "stafford", "chester"],
        ["london", "oxford", "gloucester", "hereford", "ludlow", "shrewsbury", "chester"],
        ["london", "windsor", "winchester", "southampton"],
        ["winchester", "salisbury", "exeter", "plymouth"],
        ["london", "bath", "bristol", "gloucester", "worcester", "shrewsbury"],
        ["london", "canterbury", "dover"], ["canterbury", "sandwich"],
        ["london", "colchester", "ipswich", "norwich"],
        ["london", "cambridge", "kings_lynn", "norwich"],
        ["york", "beverley", "hull"], ["york", "scarborough"],
        ["coventry", "leicester", "nottingham", "doncaster"],
        ["newark", "lincoln", "hull"],
        ["york", "skipton", "kendal", "carlisle", "newcastle"],
        ["kendal", "lancaster", "chester"],
        ["coventry", "warwick", "oxford"],
    ]
    for road in roads:
        d.line([P(i) for i in road], fill=ROAD, width=3)

    # Grid in GAME kilometres
    step_game = 5
    step_real = step_game * COMPRESSION
    f_small = font("EBGaramond-Italic", 22)
    gx0 = math.floor(x_min / step_real) * step_real
    gy0 = math.floor(y_min / step_real) * step_real
    x = gx0
    while x <= x_max:
        X = MARGIN + (x - x_min) * PX_PER_KM
        d.line([(X, MARGIN + 120), (X, H - MARGIN - 40)], fill=(90, 70, 50), width=1)
        x += step_real
    y = gy0
    while y <= y_max:
        Y = MARGIN + 120 + (y_max - y) * PX_PER_KM
        d.line([(MARGIN, Y), (W - MARGIN, Y)], fill=(90, 70, 50), width=1)
        y += step_real

    # Places
    f_city = font("Cinzel-Bold", 34)
    f_town = font("Cinzel-SemiBold", 24)
    f_minor = font("EBGaramond-SemiBold", 22)
    f_battle = font("EBGaramond-Italic", 21)
    for row in places.values():
        X, Y = px(float(row["Lon"]), float(row["Lat"]))
        kind, imp = row["Type"], int(row["Importance"])
        name = row["Name"]
        if kind in ("City", "Town"):
            s = {3: 16, 2: 12, 1: 8}[imp] * (1.25 if kind == "City" else 1)
            d.rectangle([X - s, Y - s * 0.4, X + s, Y + s * 0.9], fill=(240, 232, 214), outline=INK, width=2)
            d.polygon([(X - s * 1.15, Y - s * 0.4), (X, Y - s * 1.4), (X + s * 1.15, Y - s * 0.4)], fill=ROOF, outline=INK)
            if kind == "City":
                d.rectangle([X + s * 0.35, Y - s * 1.9, X + s * 0.8, Y - s * 0.4], fill=(240, 232, 214), outline=INK, width=2)
            fnt = f_city if imp == 3 else (f_town if imp == 2 else f_minor)
            d.text((X + s + 6, Y - s), name, font=fnt, fill=INK)
        elif kind == "Castle":
            s = {3: 11, 2: 9, 1: 7}[imp]
            d.rectangle([X - s, Y - s, X + s, Y + s], fill=(120, 110, 100), outline=INK, width=2)
            for k in (-1, 0, 1):
                d.rectangle([X + k * s * 0.7 - 2, Y - s - 5, X + k * s * 0.7 + 3, Y - s], fill=(120, 110, 100), outline=INK)
            if imp >= 2:
                d.text((X + s + 5, Y + 2), name.replace(" Castle", ""), font=f_minor, fill=(70, 60, 55))
        elif kind == "Abbey":
            d.line([(X, Y - 10), (X, Y + 10)], fill=INK, width=3)
            d.line([(X - 7, Y - 4), (X + 7, Y - 4)], fill=INK, width=3)
            if imp >= 2:
                d.text((X + 9, Y - 6), name, font=f_battle, fill=(70, 60, 55))
        elif kind == "Battle":
            r = 9 if imp < 3 else 13
            d.line([(X - r, Y - r), (X + r, Y + r)], fill=(150, 20, 20), width=4)
            d.line([(X - r, Y + r), (X + r, Y - r)], fill=(150, 20, 20), width=4)
            d.text((X - 10, Y + r + 2), name, font=f_battle, fill=(130, 20, 20))
        elif kind == "Village":
            d.ellipse([X - 4, Y - 4, X + 4, Y + 4], fill=INK)

    # Title, scale, legend
    area_game = england_area / COMPRESSION ** 2
    f_title = font("Cinzel-Bold", 64)
    d.text((MARGIN, 40), "Crowns & Commoners - England, 1455", font=f_title, fill=INK)
    d.text((MARGIN, 120), f"Planning map. Compression 1:{COMPRESSION:g} ({COMPRESSION:g} real km = 1 game km). "
                          f"England {england_area:,.0f} km2 real -> about {area_game:,.0f} km2 in game. "
                          f"Grid squares = {step_game} game km ({step_real:g} real km).",
           font=font("EBGaramond-Regular", 28), fill=INK)
    lx, ly = MARGIN, H - MARGIN + 10
    d.text((lx, ly), "Red lines: main roads  /  green: rivers  /  red roofs: towns (large = cities)  /  grey towers: castles  /"
                     "  crosses: abbeys  /  red X: battles of the war  /  dots: villages  /  faded: future DLC lands",
           font=f_small, fill=INK)

    os.makedirs(f"{ROOT}/Docs/World", exist_ok=True)
    out = f"{ROOT}/Docs/World/plan_map_england_1455.png"
    img.save(out)
    print(f"England real area {england_area:,.0f} km2; at 1:{COMPRESSION:g} -> {area_game:,.0f} km2 in game; image {W}x{H} -> {out}")


if __name__ == "__main__":
    main()
