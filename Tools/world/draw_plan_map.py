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

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont

Image.MAX_IMAGE_PIXELS = None
ROOT = "C:/Dev/CountriesIRL_3D_Game"
DATA = f"{ROOT}/Art/MapData"
COMPRESSION = float(sys.argv[1]) if len(sys.argv) > 1 else 12.0

# Projection: equirectangular around the middle of England, in real kilometres
LAT0, LON0 = 53.0, -1.9
KX = 111.32 * math.cos(math.radians(LAT0))
KY = 110.57
BOX = (-5.8, 49.85, 1.85, 55.85)     # lon/lat window drawn: just England (and Wales)
PX_PER_KM = 9.0                      # image scale (real km)
S = PX_PER_KM / 4.0                  # size factor for symbols and text (designed at 4 px/km)
MARGIN = int(110 * S)
TOP = int(150 * S)

PARCH = (236, 222, 188)
PARCH_DARK = (214, 194, 150)
SEA = (150, 178, 150)
RIVER = (78, 122, 106)
ROAD = (168, 46, 32)
INK = (58, 40, 26)
ROOF = (176, 52, 40)
FADED = (196, 192, 176)


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


def elevation_layer():
    """Elevation (metres) resampled onto the map image, at quarter resolution, or None if not downloaded."""
    meta_path = f"{DATA}/elevation_z8.json"
    if not os.path.exists(meta_path):
        return None
    meta = json.load(open(meta_path))
    tiles = Image.open(f"{DATA}/elevation_z8.png").convert("RGB")
    n = 2 ** meta["z"]
    tw, th = tiles.size
    src = tiles.load()
    w, h = W // 4, H // 4
    out = Image.new("F", (w, h))
    o = out.load()
    for j in range(h):
        for i in range(w):
            lon, lat = unpx(i * 4 + 2, j * 4 + 2)
            fx = ((lon + 180) / 360 * n - meta["x0"]) * 256
            fy = ((1 - math.log(math.tan(math.radians(lat)) + 1 / math.cos(math.radians(lat))) / math.pi) / 2 * n - meta["y0"]) * 256
            if 0 <= fx < tw and 0 <= fy < th:
                r, g, b = src[int(fx), int(fy)]
                o[i, j] = max((r * 256 + g + b / 256) - 32768, 0)
    return out


def hillshade(elev):
    """Soft shading lit from the north-west, as a greyscale multiply layer (255 = no change)."""
    w, h = elev.size
    e = elev.load()
    shade = Image.new("L", (w, h), 255)
    s = shade.load()
    cell_m = 4 / PX_PER_KM * 1000 / 3.0      # exaggerate relief 3x so it reads on paper
    for j in range(1, h - 1):
        for i in range(1, w - 1):
            dzdx = (e[i + 1, j] - e[i - 1, j]) / (2 * cell_m)
            dzdy = (e[i, j + 1] - e[i, j - 1]) / (2 * cell_m)
            light = (-dzdx - dzdy) / math.sqrt(2)          # light from the top-left
            s[i, j] = int(max(150, min(255, 225 + light * 220)))
    return shade.filter(ImageFilter.GaussianBlur(1))


def main():
    rnd = random.Random(3)

    # Sea with soft painted blotches
    img = Image.new("RGB", (W, H), SEA)
    blot = Image.new("L", (W // 40, H // 40))
    blot.putdata([rnd.randint(0, 45) for _ in range(blot.width * blot.height)])
    blot = blot.resize((W, H), Image.BICUBIC).filter(ImageFilter.GaussianBlur(30))
    img = Image.composite(Image.new("RGB", (W, H), (124, 158, 132)), img, blot)

    # Land: England bright, Wales faded (future DLC)
    land = Image.new("L", (W, H), 0)
    ld = ImageDraw.Draw(land)
    d = ImageDraw.Draw(img)
    units = json.load(open(f"{DATA}/ne_10m_admin_0_map_units.geojson", encoding="utf-8"))
    england_area = 0.0
    for f in units["features"]:
        name = f["properties"].get("NAME")
        if name not in ("England", "Wales"):
            continue
        for poly in geo_polys(f["geometry"]):
            ring = [px(lon, lat) for lon, lat in poly[0]]
            d.polygon(ring, fill=PARCH if name == "England" else FADED)
            if name == "England":
                ld.polygon(ring, fill=255)
                pts = [km(lon, lat) for lon, lat in poly[0]]
                england_area += abs(sum(pts[i][0] * pts[i - 1][1] - pts[i - 1][0] * pts[i][1] for i in range(len(pts)))) / 2

    # Parchment grain on England
    grain = Image.new("L", (W // 14, H // 14))
    grain.putdata([rnd.randint(0, 255) for _ in range(grain.width * grain.height)])
    grain = grain.resize((W, H), Image.BICUBIC).filter(ImageFilter.GaussianBlur(8)).point(lambda v: 60 if v > 150 else 0)
    img = Image.composite(Image.new("RGB", (W, H), PARCH_DARK), img, ImageChops.multiply(grain, land))

    # Hills: shaded relief plus a brownish tint on high ground
    elev = elevation_layer()
    if elev is not None:
        shade = hillshade(elev).resize((W, H), Image.BICUBIC)
        high = elev.point(lambda v: v * (255 / 600.0)).convert("L").resize((W, H), Image.BICUBIC).filter(ImageFilter.GaussianBlur(3))
        img = Image.composite(Image.new("RGB", (W, H), (190, 160, 118)), img, ImageChops.multiply(high.point(lambda v: v * 0.55), land))
        shaded = ImageChops.multiply(img, Image.merge("RGB", (shade, shade, shade)))
        img = Image.composite(shaded, img, land)

    d = ImageDraw.Draw(img)
    # Coastline outline
    for f in units["features"]:
        if f["properties"].get("NAME") in ("England", "Wales"):
            for poly in geo_polys(f["geometry"]):
                d.line([px(lon, lat) for lon, lat in poly[0]] + [px(*poly[0][0])], fill=INK, width=max(2, int(1.5 * S)))

    # Rivers: OpenStreetMap main rivers if downloaded, otherwise Natural Earth
    river_files = ["osm_rivers_england"] if os.path.exists(f"{DATA}/osm_rivers_england.geojson") else \
        ["ne_10m_rivers_lake_centerlines", "ne_10m_rivers_europe"]
    for fname in river_files:
        rivers = json.load(open(f"{DATA}/{fname}.geojson", encoding="utf-8"))
        for f in rivers["features"]:
            for line in geo_lines(f["geometry"]):
                if not any(in_box(lon, lat) for lon, lat in line) or all(lat < 51.0 and lon > 0.5 for lon, lat in line):
                    continue
                d.line([px(lon, lat) for lon, lat in line], fill=RIVER, width=max(2, int(1.6 * S)), joint="curve")

    places = {row["Id"]: row for row in csv.DictReader(open(f"{ROOT}/Data/World/Places_England1455.csv", encoding="utf-8"))}
    P = lambda i: px(float(places[i]["Lon"]), float(places[i]["Lat"]))

    # Main roads of the period (after the Gough Map's red routes)
    roads = [
        ["london", "ware", "huntingdon", "stamford", "grantham", "newark", "doncaster", "pontefract", "tadcaster", "york"],
        ["york", "ripon", "northallerton", "darlington", "durham", "newcastle", "morpeth", "alnwick", "berwick"],
        ["ripon", "middleham", "richmond", "barnard_castle", "appleby", "penrith", "carlisle"],
        ["london", "st_albans", "dunstable", "northampton", "coventry", "lichfield", "stafford", "chester"],
        ["london", "reading", "abingdon", "oxford", "cirencester", "gloucester", "hereford", "ludlow", "shrewsbury", "chester"],
        ["london", "kingston", "guildford", "winchester", "southampton"],
        ["winchester", "salisbury", "dorchester", "exeter", "plymouth"],
        ["exeter", "launceston", "bodmin", "truro"],
        ["reading", "marlborough", "bath", "bristol", "gloucester", "worcester", "shrewsbury"],
        ["london", "rochester", "canterbury", "dover"], ["canterbury", "sandwich"],
        ["london", "chelmsford", "colchester", "ipswich", "norwich", "yarmouth"],
        ["london", "cambridge", "ely", "kings_lynn", "norwich"],
        ["york", "beverley", "hull"], ["york", "malton", "scarborough"],
        ["coventry", "leicester", "nottingham", "doncaster"],
        ["newark", "lincoln", "louth", "grimsby"],
        ["york", "knaresborough", "skipton", "kendal", "penrith"],
        ["kendal", "lancaster", "preston", "chester"],
        ["coventry", "warwick", "banbury", "oxford"],
        ["newcastle", "hexham", "carlisle"],
        ["bristol", "bridgwater", "taunton", "exeter"],
        ["london", "lewes"], ["guildford", "arundel", "chichester", "portsmouth"],
    ]
    for road in roads:
        d.line([P(i) for i in road if i in places], fill=ROAD, width=max(2, int(1.8 * S)))

    # Grid in GAME kilometres
    step_game = 5
    step_real = step_game * COMPRESSION
    x = math.floor(x_min / step_real) * step_real
    while x <= x_max:
        X = MARGIN + (x - x_min) * PX_PER_KM
        d.line([(X, TOP), (X, H - MARGIN)], fill=(90, 70, 50), width=1)
        x += step_real
    y = math.floor(y_min / step_real) * step_real
    while y <= y_max:
        Y = TOP + (y_max - y) * PX_PER_KM
        d.line([(MARGIN, Y), (W - MARGIN, Y)], fill=(90, 70, 50), width=1)
        y += step_real

    # Region names first (under everything else)
    f_area = font("EBGaramond-Italic", 30)
    f_area_big = font("EBGaramond-Italic", 40)
    for row in places.values():
        if row["Type"] == "Area":
            X, Y = px(float(row["Lon"]), float(row["Lat"]))
            fnt = f_area_big if int(row["Importance"]) >= 2 else f_area
            text = row["Name"].upper() if int(row["Importance"]) >= 2 else row["Name"]
            w_ = d.textlength(text, font=fnt)
            d.text((X - w_ / 2, Y), text, font=fnt, fill=(110, 84, 58))

    # Places
    f_city = font("Cinzel-Bold", 30)
    f_town = font("Cinzel-SemiBold", 21)
    f_minor = font("EBGaramond-SemiBold", 17)
    f_small = font("EBGaramond-Italic", 15)
    f_battle = font("EBGaramond-Italic", 17)
    order = {"Village": 0, "Abbey": 1, "Nature": 2, "Landmark": 3, "Castle": 4, "Cathedral": 5, "Battle": 6, "Town": 7, "City": 8}
    for row in sorted(places.values(), key=lambda r: (order.get(r["Type"], 0), int(r["Importance"]))):
        kind, imp = row["Type"], int(row["Importance"])
        if kind == "Area":
            continue
        X, Y = px(float(row["Lon"]), float(row["Lat"]))
        name = row["Name"]
        if kind in ("City", "Town"):
            s = {3: 13, 2: 10, 1: 6.5}[imp] * (1.25 if kind == "City" else 1) * S
            d.rectangle([X - s, Y - s * 0.4, X + s, Y + s * 0.9], fill=(240, 232, 214), outline=INK, width=2)
            d.polygon([(X - s * 1.15, Y - s * 0.4), (X, Y - s * 1.4), (X + s * 1.15, Y - s * 0.4)], fill=ROOF, outline=INK)
            if kind == "City":
                d.rectangle([X + s * 0.35, Y - s * 1.9, X + s * 0.8, Y - s * 0.4], fill=(240, 232, 214), outline=INK, width=2)
            fnt = f_city if imp == 3 else (f_town if imp == 2 else f_minor)
            d.text((X + s + 4 * S, Y - s), name, font=fnt, fill=INK)
        elif kind == "Castle":
            s = {3: 7, 2: 5.5, 1: 3.5}[imp] * S
            d.rectangle([X - s, Y - s, X + s, Y + s], fill=(118, 108, 98), outline=INK, width=max(1, int(S)))
            if imp >= 2:
                for k in (-1, 0, 1):
                    d.rectangle([X + k * s * 0.7 - s * 0.25, Y - s * 1.45, X + k * s * 0.7 + s * 0.25, Y - s], fill=(118, 108, 98), outline=INK)
                d.text((X + s + 3 * S, Y), name.replace(" Castle", ""), font=f_minor, fill=(70, 60, 55))
        elif kind in ("Abbey", "Cathedral"):
            s = (8 if kind == "Cathedral" else (5 if imp >= 2 else 3.2)) * S
            col = INK if kind == "Cathedral" or imp >= 2 else (95, 75, 60)
            d.line([(X, Y - s), (X, Y + s)], fill=col, width=max(2, int(s / 3)))
            d.line([(X - s * 0.7, Y - s * 0.35), (X + s * 0.7, Y - s * 0.35)], fill=col, width=max(2, int(s / 3)))
            if imp >= 2:
                d.text((X + s + 2 * S, Y - s), name, font=f_small, fill=(70, 60, 55))
        elif kind == "Landmark":
            s = 6 * S
            for k in (-1, 0, 1):   # three standing stones
                d.rectangle([X + k * s * 0.8 - s * 0.25, Y - s * 0.9, X + k * s * 0.8 + s * 0.25, Y + s * 0.5], fill=(150, 140, 125), outline=INK)
            d.text((X + s * 1.4, Y - s), name, font=f_battle if imp >= 2 else f_small, fill=(60, 50, 90))
        elif kind == "Nature":
            s = 6 * S
            d.polygon([(X - s, Y + s * 0.6), (X, Y - s * 0.9), (X + s, Y + s * 0.6)], fill=(120, 150, 100), outline=INK)
            d.text((X + s + 2 * S, Y - s * 0.6), name, font=f_small, fill=(40, 80, 50))
        elif kind == "Battle":
            r = (6 if imp < 3 else 9) * S
            d.line([(X - r, Y - r), (X + r, Y + r)], fill=(150, 20, 20), width=max(2, int(1.6 * S)))
            d.line([(X - r, Y + r), (X + r, Y - r)], fill=(150, 20, 20), width=max(2, int(1.6 * S)))
            d.text((X - 8 * S, Y + r + 1 * S), name, font=f_battle, fill=(130, 20, 20))
        elif kind == "Village":
            r = 2.2 * S
            d.ellipse([X - r, Y - r, X + r, Y + r], fill=INK)
            d.text((X + r + 2 * S, Y - r * 2), name, font=f_small, fill=(80, 64, 48))

    # Title, scale, legend
    area_game = england_area / COMPRESSION ** 2
    counts = {}
    for r in places.values():
        counts[r["Type"]] = counts.get(r["Type"], 0) + 1
    d.text((MARGIN, int(30 * S)), "Crowns & Commoners - England, 1455", font=font("Cinzel-Bold", 56), fill=INK)
    d.text((MARGIN, int(95 * S)), f"Planning map. Compression 1:{COMPRESSION:g} ({COMPRESSION:g} real km = 1 game km). "
                                  f"England {england_area:,.0f} km2 real -> about {area_game:,.0f} km2 in game. "
                                  f"Grid squares = {step_game} game km ({step_real:g} real km).",
           font=font("EBGaramond-Regular", 24), fill=INK)
    legend = (f"Towns & cities {counts.get('Town', 0) + counts.get('City', 0)}  /  castles {counts.get('Castle', 0)}  /  "
              f"abbeys, priories & friaries {counts.get('Abbey', 0)}  /  cathedrals {counts.get('Cathedral', 0)}  /  "
              f"landmarks {counts.get('Landmark', 0)}  /  nature {counts.get('Nature', 0)}  /  battles {counts.get('Battle', 0)}  /  "
              f"villages {counts.get('Village', 0)}.   Red lines: roads. Wales faded: future DLC. "
              f"Data: Natural Earth, OpenStreetMap contributors, Wikidata, AWS Terrain Tiles.")
    d.text((MARGIN, H - MARGIN + int(10 * S)), legend, font=font("EBGaramond-Italic", 18), fill=INK)

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
    print(f"England real area {england_area:,.0f} km2; at 1:{COMPRESSION:g} -> {area_game:,.0f} km2 in game; image {W}x{H}; {counts}")


if __name__ == "__main__":
    main()
