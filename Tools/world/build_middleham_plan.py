"""
The plan of Middleham in 1455 for the game: the real place (OpenStreetMap today + real heights + what the history
books say about 1455) squeezed into the 1 x 1 km test landscape, and drawn as the game's own town map.

How it's squeezed: distances from the castle are shrunk more the further out they are. Close to the castle (the
town) about 1:1.4, far out (the rivers, the parks) about 1:5, averaging the towns' 1:3: 1.5 km of the real place
fits in the 500 m from the castle to the edge. Directions stay true. Buildings, the castle, the church, roads and
people keep their REAL size: squeezing the town leaves room for fewer of them (a greedy pass keeps the ones that
still fit). Modern things are left out; things from after 1455 too.

Writes
  Data/World/Middleham/Middleham1455.json   the plan in game metres (origin = castle, +x east, +y north)
  Data/World/Middleham/Middleham1455_height.png  16-bit game heights (for the terrain), 1025 x 1025 over the 1 km
  Docs/World/Middleham/middleham_1455_plan.png   the plan with a title, scale and legend, for checking
  Art/UI/Maps/localmap_middleham.png         the same map without margins: the in-game town map
    python Tools/world/build_middleham_plan.py
Sources: OpenStreetMap ((c) OpenStreetMap contributors, ODbL), AWS Terrain Tiles, Victoria County History (North
Riding vol. 1, Middleham), English Heritage (castle history).
"""
import json, math, os
import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageFont

LAT0, LON0 = 54.2838, -1.8066                   # the reference point the downloads were centred on
KX, KY = 111320 * math.cos(math.radians(LAT0)), 111320.0
HALF = 500.0                                    # game metres from the centre to the edge
# Squeeze: game distance = integral of S(r) from the castle, S(r) = S_FAR + (S_NEAR - S_FAR) * exp(-r / FALL)
S_NEAR, S_FAR, FALL = 0.7, 0.2, 400.0

osm = json.load(open("Art/MapData/Middleham/osm_middleham.json"))
elev = np.load("Art/MapData/Middleham/elevation_middleham.npy")      # 3.2 x 3.2 km around the reference, north up
ELEV_HALF = 1600.0


def real_m(lat, lon):
    return np.array([(lon - LON0) * KX, (lat - LAT0) * KY])


# The castle is the centre of everything
castle_way = next(e for e in osm["elements"] if e.get("tags", {}).get("historic") == "castle")
castle_real = np.array([real_m(p["lat"], p["lon"]) for p in castle_way["geometry"]])
CENTRE = (castle_real.min(axis=0) + castle_real.max(axis=0)) / 2


def squeeze(r):
    return S_FAR * r + (S_NEAR - S_FAR) * FALL * (1 - np.exp(-r / FALL))


_r_table = np.linspace(0, 4000, 8001)
_g_table = squeeze(_r_table)


def unsqueeze(g):
    return np.interp(g, _g_table, _r_table)


def to_game(p_real):
    d = np.asarray(p_real, dtype=float) - CENTRE
    r = np.hypot(d[..., 0], d[..., 1])
    scale = np.where(r > 1e-6, squeeze(r) / np.maximum(r, 1e-6), S_NEAR)
    return d * scale[..., None] if d.ndim > 1 else d * scale


def to_real(p_game):
    g = np.asarray(p_game, dtype=float)
    rg = np.hypot(g[..., 0], g[..., 1])
    scale = np.where(rg > 1e-6, unsqueeze(rg) / np.maximum(rg, 1e-6), 1 / S_NEAR)
    return CENTRE + g * scale[..., None]


def geo_to_game(lat, lon):
    return to_game(real_m(lat, lon))


def inside(p, margin=0.0):
    return abs(p[0]) <= HALF - margin and abs(p[1]) <= HALF - margin


# ---- Heights: the real ground under each game point, squashed like the rest of England's terrain -------------------
CURVE_REAL = [0, 60, 150, 300, 500, 700, 1000]
CURVE_GAME = [0, 6, 22, 55, 120, 190, 280]
N = 1025
gx, gy = np.meshgrid(np.linspace(-HALF, HALF, N), np.linspace(HALF, -HALF, N))
real = to_real(np.stack([gx, gy], axis=-1))
col = (real[..., 0] + ELEV_HALF) / (2 * ELEV_HALF) * (elev.shape[1] - 1)
row = (ELEV_HALF - real[..., 1]) / (2 * ELEV_HALF) * (elev.shape[0] - 1)
c0, r0 = np.clip(np.floor(col).astype(int), 0, elev.shape[1] - 2), np.clip(np.floor(row).astype(int), 0, elev.shape[0] - 2)
fc, fr = np.clip(col - c0, 0, 1), np.clip(row - r0, 0, 1)
real_h = (elev[r0, c0] * (1 - fc) * (1 - fr) + elev[r0, c0 + 1] * fc * (1 - fr)
          + elev[r0 + 1, c0] * (1 - fc) * fr + elev[r0 + 1, c0 + 1] * fc * fr)
game_h = np.interp(real_h, CURVE_REAL, CURVE_GAME)
os.makedirs("Data/World/Middleham", exist_ok=True)
H_MIN, H_MAX = float(game_h.min()), float(game_h.max())
Image.fromarray(((game_h - H_MIN) / (H_MAX - H_MIN) * 65535).astype(np.uint16)).save("Data/World/Middleham/Middleham1455_height.png")


def height_at(p):
    c = int(round((p[0] + HALF) / (2 * HALF) * (N - 1))); r = int(round((HALF - p[1]) / (2 * HALF) * (N - 1)))
    return float(game_h[min(max(r, 0), N - 1), min(max(c, 0), N - 1)])


# ---- Lines: rivers, streams, the roads of 1455 ----------------------------------------------------------------------
def way_game(el):
    return [geo_to_game(p["lat"], p["lon"]) for p in el["geometry"]]


def clip_line(pts, margin=-40.0):
    """Keeps the parts of a line inside the area (with a little overhang so lines run off the edge)"""
    parts, cur = [], []
    for p in pts:
        if inside(p, margin):
            cur.append([round(float(p[0]), 1), round(float(p[1]), 1)])
        elif cur:
            parts.append(cur); cur = []
    if cur:
        parts.append(cur)
    return [c for c in parts if len(c) > 1]


# Roads that existed in 1455 (today's names): the Ripon - Hawes road through the town, the Coverdale road, the old
# lanes of the town and out to the fields and fords. Modern estates, drives and stable yards are left out
ROADS_1455 = {
    "Leyburn Road": ("road", 6.0), "East Witton Road": ("road", 6.0), "Kirkgate": ("road", 6.0), "Market Place": ("road", 6.0),
    "Coverham Lane": ("road", 5.0), "Park Lane": ("lane", 4.0), "Back Street": ("lane", 3.5), "Church Street": ("lane", 3.5),
    "Castle Hill": ("lane", 3.0), "Middleham Lane": ("track", 3.0), "Straight Lane": ("track", 3.0), "Green Gate": ("track", 3.0),
    "West Field": ("track", 3.0), "Braithwaite Lane": ("lane", 4.0), "Hargill Road": ("lane", 4.0), "Wood Lane": ("lane", 4.0),
    "Low Lane": ("lane", 4.0), "Mighten's Bank": ("road", 6.0),
}
plan = {"note": "Middleham in 1455, game metres from the castle (+x east, +y north). Buildings at real size.",
        "half_size_m": HALF, "height_range_m": [round(H_MIN, 2), round(H_MAX, 2)],
        "rivers": [], "streams": [], "roads": [], "buildings": [], "areas": [], "points": []}
for el in osm["elements"]:
    if el["type"] != "way" or "geometry" not in el:
        continue
    t = el.get("tags", {})
    if t.get("waterway") == "river":
        width = 12.0 if t.get("name") == "River Ure" else 7.0          # about a third of the real width
        for part in clip_line(way_game(el)):
            plan["rivers"].append({"name": t.get("name"), "width_m": width, "points": part})
    elif t.get("waterway") == "stream":
        for part in clip_line(way_game(el)):
            plan["streams"].append({"name": t.get("name"), "width_m": 2.0, "points": part})
    elif t.get("name") in ROADS_1455 and "highway" in t and t.get("highway") != "residential" or t.get("name") in ("Back Street", "Church Street") and "highway" in t:
        kind, width = ROADS_1455[t["name"]]
        for part in clip_line(way_game(el)):
            plan["roads"].append({"name": t["name"], "kind": kind, "width_m": width, "points": part})


# ---- Buildings ------------------------------------------------------------------------------------------------------
def footprint(el):
    """Centre (real), length, width, angle of a building's outline (its longest side)"""
    pts = np.array([real_m(p["lat"], p["lon"]) for p in el["geometry"]])
    c = pts.mean(axis=0)
    best = (0, 0.0)
    for a, b in zip(pts[:-1], pts[1:]):
        L = np.hypot(*(b - a))
        if L > best[0]:
            best = (L, math.atan2(b[1] - a[1], b[0] - a[0]))
    ang = best[1]
    u = np.array([math.cos(ang), math.sin(ang)]); v = np.array([-u[1], u[0]])
    lu, lv = (pts - c) @ u, (pts - c) @ v
    return c, lu.max() - lu.min(), lv.max() - lv.min(), ang


def rect(c, length, width, ang):
    u = np.array([math.cos(ang), math.sin(ang)]) * length / 2; v = np.array([-math.sin(ang), math.cos(ang)]) * width / 2
    return [c + u + v, c - u + v, c - u - v, c + u - v]


def overlaps(a, b, gap):
    """Separating-axis test for two convex outlines, with a gap between them"""
    for poly in (a, b):
        for i in range(len(poly)):
            e = poly[(i + 1) % len(poly)] - poly[i]
            n = np.array([-e[1], e[0]]) / max(np.hypot(*e), 1e-6)
            pa, pb = [p @ n for p in a], [p @ n for p in b]
            if max(pa) + gap < min(pb) or max(pb) + gap < min(pa):
                return False
    return True


placed = []      # outlines (game) that new buildings must keep clear of
# The castle and the churches at real size, where their centres fall in the game
castle_game = [to_game(CENTRE) + (p - CENTRE) for p in castle_real]
plan["buildings"].append({"kind": "castle", "name": "Middleham Castle", "outline": [[round(float(x), 1), round(float(y), 1)] for x, y in castle_game]})
placed.append(np.array([castle_game[i] for i in range(0, len(castle_game), max(1, len(castle_game) // 12))]))
for el in osm["elements"]:
    t = el.get("tags", {})
    if t.get("amenity") == "place_of_worship" and el["type"] == "way":
        c, L, W, a = footprint(el)
        g = to_game(c)
        if inside(g, 20):
            outline = rect(g, L, W, a)
            plan["buildings"].append({"kind": "church", "name": t.get("name"), "centre": [round(float(g[0]), 1), round(float(g[1]), 1)],
                                      "length_m": round(float(L), 1), "width_m": round(float(W), 1), "angle_deg": round(math.degrees(a), 1),
                                      "height_m": round(height_at(g), 2)})
            placed.append(np.array(outline))

# Road corridors houses must not stand on
road_segments = [(np.array(a), np.array(b), r["width_m"]) for r in plan["roads"] for a, b in zip(r["points"][:-1], r["points"][1:])]


def on_road(outline, clearance=1.5):
    pts = list(outline) + [np.mean(outline, axis=0)]
    for a, b, w in road_segments:
        ab = b - a; L2 = ab @ ab
        if L2 < 1e-6:
            continue
        for p in pts:
            t = np.clip((p - a) @ ab / L2, 0, 1)
            if np.hypot(*(p - (a + t * ab))) < w / 2 + clearance:
                return True
    return False


# Houses: today's buildings in the old town (the streets around the two market places) and in Spennithorne stand on
# old plots. Medieval houses: long and narrow, 8-14 m by 5-7 m. Kept only where they still fit once squeezed
OLD_TOWN = [(np.array([45.0, 150.0]), 230.0), (np.array([-60.0, 100.0]), 150.0)]         # real metres from the reference
SPENNITHORNE = (np.array([1000.0, 1400.0]), 250.0)
candidates = []
for el in osm["elements"]:
    t = el.get("tags", {})
    if el["type"] != "way" or "building" not in t or "geometry" not in el or t.get("amenity") == "place_of_worship":
        continue
    c, L, W, a = footprint(el)
    in_town = any(np.hypot(*(c - p)) < r for p, r in OLD_TOWN)
    in_village = np.hypot(*(c - SPENNITHORNE[0])) < SPENNITHORNE[1]
    if not (in_town or in_village) or L * W < 25:
        continue
    candidates.append((c, L, W, a, "town" if in_town else "Spennithorne"))
# Near the market places first, so the town keeps its heart
markets_real = [np.array([65.0, 172.0]), np.array([-83.0, 89.0])]
candidates.sort(key=lambda c: min(np.hypot(*(c[0] - m)) for m in markets_real))
counts = {"town": 0, "Spennithorne": 0}
LIMITS = {"town": 34, "Spennithorne": 8}
for c, L, W, a, where in candidates:
    if counts[where] >= LIMITS[where]:
        continue
    g = to_game(c)
    L2, W2 = min(max(L, 8.0), 14.0), min(max(W, 5.0), 7.0)
    outline = [np.array(p) for p in rect(g, L2, W2, a)]
    if not inside(g, 15) or on_road(outline) or any(overlaps(outline, list(o), 2.5) for o in placed):
        continue
    placed.append(np.array(outline))
    counts[where] += 1
    plan["buildings"].append({"kind": "house", "place": where, "centre": [round(float(g[0]), 1), round(float(g[1]), 1)],
                              "length_m": round(L2, 1), "width_m": round(W2, 1), "angle_deg": round(math.degrees(a), 1),
                              "height_m": round(height_at(g), 2)})


# ---- Areas: the deer parks, woods, market places ---------------------------------------------------------------------
def area(name, kind, real_pts):
    pts = [to_game(np.array(p, dtype=float) + CENTRE) for p in real_pts]
    plan["areas"].append({"name": name, "kind": kind, "outline": [[round(float(x), 1), round(float(y), 1)] for x, y in pts]})


# The Nevilles' two deer parks (licensed 1335): Sunskew (East) Park south of the town around William's Hill, down to
# the Cover; West Park to the north-west. Wooded pasture inside a pale (a paling fence on a bank)
area("Sunskew Park", "park", [(-700, -160), (-150, -120), (500, -140), (950, -450), (800, -900), (100, -1000), (-600, -950), (-950, -600)])
area("West Park", "park", [(-1700, 150), (-450, 120), (-300, 450), (-650, 850), (-1250, 950), (-1800, 700)])
for el in osm["elements"]:
    t = el.get("tags", {})
    if el["type"] == "way" and (t.get("landuse") == "forest" or t.get("natural") == "wood") and "geometry" in el:
        pts = way_game(el)
        if any(inside(p) for p in pts):
            plan["areas"].append({"name": t.get("name"), "kind": "wood", "outline": [[round(float(p[0]), 1), round(float(p[1]), 1)] for p in pts]})


def square(name, real_centre, w, h, ang=0.0):
    g = to_game(np.array(real_centre, dtype=float))
    plan["areas"].append({"name": name, "kind": "market", "outline": [[round(float(p[0]), 1), round(float(p[1]), 1)] for p in rect(g, w, h, math.radians(ang))]})


# Two market places (market charter 1389): the lower one to the north with its stone cross, the upper swine market
square("Market Place", (65, 172), 34, 22, 20)
square("Swine Market", (-83, 89), 26, 16, 0)


# ---- Points: crossings, mill, cross, well, William's Hill ------------------------------------------------------------
def point(name, kind, p_game, note=""):
    plan["points"].append({"name": name, "kind": kind, "position": [round(float(p_game[0]), 1), round(float(p_game[1]), 1)], "note": note})


def nearest_on(lines, target):
    best, bp = 1e9, None
    for line in lines:
        for p in line["points"]:
            d = np.hypot(p[0] - target[0], p[1] - target[1])
            if d < best:
                best, bp = d, p
    return np.array(bp)


ure = [r for r in plan["rivers"] if r["name"] == "River Ure"]
cover = [r for r in plan["rivers"] if r["name"] == "River Cover"]
point("Market cross", "cross", to_game(np.array([65.0, 172.0])), "medieval cross in the lower market place")
point("St Alkelda's Well", "well", to_game(np.array([-75.0, 262.0])), "holy well by the church")
point("William's Hill", "earthwork", to_game(np.array([-208.0, -313.0])), "the Norman ringwork of about 1086, a grassy earthwork by 1455")
if ure:
    point("Ure ford", "ford", nearest_on(ure, to_game(np.array([-786.0, 1180.0]))), "where the Leyburn road crosses (the bridge came in 1829)")
if cover:
    mill = nearest_on(cover, to_game(np.array([-150.0, -1040.0])))
    point("Middleham Mill", "mill", mill, "water mill on the Cover (Leaze mill, gone before 1672)")
    point("Cover ford", "ford", nearest_on(cover, to_game(np.array([60.0, -1000.0]))), "")

json.dump(plan, open("Data/World/Middleham/Middleham1455.json", "w"), indent=1)
print("plan: %d river parts, %d roads, %d buildings (%d houses in town, %d in Spennithorne), %d areas, height %.1f-%.1f m"
      % (len(plan["rivers"]), len(plan["roads"]), len(plan["buildings"]), counts["town"], counts["Spennithorne"], len(plan["areas"]), H_MIN, H_MAX))

# ---- Drawing: the game's town map ------------------------------------------------------------------------------------
PX = 2048                        # pixels for the 1 km (2 px per metre)
PARCH = np.array([247, 239, 212]); FIELD = np.array([238, 228, 190]); PARK = (206, 214, 170)
RIVER = (78, 122, 106); ROAD = (92, 58, 32); LANE = (150, 112, 74); INK = (58, 40, 26); ROOF = (176, 52, 40)
STONE = (168, 160, 146); CONTOUR = (150, 118, 80)
FONT = "Art/Fonts/"


def P(p):
    return ((p[0] + HALF) / (2 * HALF) * PX, (HALF - p[1]) / (2 * HALF) * PX)


def font(name, size):
    return ImageFont.truetype(FONT + name + ".ttf", size)


# Paper with soft hill shading (light from the north-west) and a faint paper grain
hs = np.array(Image.fromarray(game_h.astype(np.float32)).resize((PX, PX), Image.BICUBIC))
dy, dx = np.gradient(hs, 2 * HALF / PX)
shade = np.clip(1.0 + (-dx + dy) * 1.4, 0.82, 1.08)
rng = np.random.default_rng(7)
grain = np.asarray(Image.fromarray((rng.random((PX // 4, PX // 4)) * 255).astype(np.uint8)).resize((PX, PX), Image.BICUBIC), dtype=np.float32) / 255
paper = PARCH[None, None, :] * shade[..., None] * (0.97 + 0.05 * grain[..., None])
img = Image.fromarray(np.clip(paper, 0, 255).astype(np.uint8)).convert("RGBA")
d = ImageDraw.Draw(img, "RGBA")

# Open fields: long strips (ridge and furrow) outside the parks and the town, tinted by turns
parks = [a for a in plan["areas"] if a["kind"] == "park"]
field_layer = Image.new("RGBA", (PX, PX), (0, 0, 0, 0)); fd = ImageDraw.Draw(field_layer)
strip = 18.0
for i, y in enumerate(np.arange(-HALF, HALF, strip)):
    tint = (214, 196, 140, 55) if i % 2 else (228, 212, 160, 40)
    fd.rectangle([P((-HALF, y + strip)), P((HALF, y))], fill=tint)
mask = Image.new("L", (PX, PX), 255); md = ImageDraw.Draw(mask)
for a in parks:
    md.polygon([P(p) for p in a["outline"]], fill=0)
md.ellipse([P((-130, 200)), P((130, -40))], fill=0)                # the town and castle stand clear of the fields
md.ellipse([P((250, 530)), P((420, 360))], fill=0)                 # and Spennithorne
img.paste(field_layer, (0, 0), Image.composite(field_layer, Image.new("RGBA", (PX, PX)), mask).getchannel("A"))
d = ImageDraw.Draw(img, "RGBA")

# Contours every 2 game metres
levels = np.arange(math.ceil(H_MIN / 2) * 2, H_MAX, 2)
for lv in levels:
    m_ = hs >= lv
    edge = (m_ ^ np.roll(m_, 1, 0)) | (m_ ^ np.roll(m_, 1, 1))
    ys, xs = np.nonzero(edge)
    a_ = 110 if int(lv) % 10 == 0 else 55
    for x, y in zip(xs[::3], ys[::3]):
        d.point((x, y), fill=CONTOUR + (a_,))

# Deer parks: pasture with trees, inside a pale
for a in parks:
    pts = [P(p) for p in a["outline"]]
    d.polygon(pts, fill=PARK + (110,))
for a in [x for x in plan["areas"] if x["kind"] in ("park", "wood")]:
    pts = [P(p) for p in a["outline"]]
    poly_mask = Image.new("L", (PX, PX), 0); ImageDraw.Draw(poly_mask).polygon(pts, fill=255)
    pm = np.asarray(poly_mask)
    count = 900 if a["kind"] == "park" else 400
    xs, ys = rng.integers(0, PX, count * 3), rng.integers(0, PX, count * 3)
    keep = pm[ys, xs] > 0
    density = 1.0 if a["kind"] == "wood" else 0.45
    for x, y in list(zip(xs[keep], ys[keep]))[: int(count * density)]:
        r = rng.uniform(7, 13)
        d.ellipse([x - r, y - r, x + r, y + r], fill=(116, 140, 88, 200), outline=(70, 90, 55, 220), width=2)
for a in parks:
    pts = [P(p) for p in a["outline"]] + [P(a["outline"][0])]
    for p0, p1 in zip(pts[:-1], pts[1:]):                       # the pale: a dashed brown line
        L_ = math.hypot(p1[0] - p0[0], p1[1] - p0[1]); n = max(int(L_ / 16), 1)
        for k in range(0, n, 2):
            a0, a1 = k / n, min((k + 1) / n, 1)
            d.line([(p0[0] + (p1[0] - p0[0]) * a0, p0[1] + (p1[1] - p0[1]) * a0), (p0[0] + (p1[0] - p0[0]) * a1, p0[1] + (p1[1] - p0[1]) * a1)], fill=(110, 80, 50, 230), width=4)

# Rivers and streams (a darker bank line under the water)
for s_ in plan["streams"]:
    d.line([P(p) for p in s_["points"]], fill=RIVER + (230,), width=4, joint="curve")
for r in plan["rivers"]:
    w = r["width_m"] * PX / (2 * HALF)
    d.line([P(p) for p in r["points"]], fill=(52, 88, 76, 255), width=int(w + 6), joint="curve")
    d.line([P(p) for p in r["points"]], fill=RIVER + (255,), width=int(w), joint="curve")

# Market places, roads (road edges in dark ink, the surface lighter)
for a in plan["areas"]:
    if a["kind"] == "market":
        d.polygon([P(p) for p in a["outline"]], fill=(222, 200, 160, 255), outline=ROAD + (255,), width=2)
for kind, colour in (("track", LANE), ("lane", LANE), ("road", ROAD)):
    for r in plan["roads"]:
        if r["kind"] != kind:
            continue
        w = max(r["width_m"] * PX / (2 * HALF), 4)
        pts = [P(p) for p in r["points"]]
        if kind == "track":
            d.line(pts, fill=colour + (200,), width=int(w * 0.6), joint="curve")
        else:
            d.line(pts, fill=colour + (255,), width=int(w + 4), joint="curve")
            d.line(pts, fill=(226, 204, 158, 255), width=int(w - 2), joint="curve")

# Buildings: houses red-roofed, churches and the castle in stone
for b in plan["buildings"]:
    if b["kind"] == "castle":
        pts = [P(p) for p in b["outline"]]
        d.polygon(pts, fill=STONE + (255,), outline=INK + (255,), width=5)
    else:
        c = np.array(b["centre"]); outline = [P(p) for p in rect(c, b["length_m"], b["width_m"], math.radians(b["angle_deg"]))]
        d.polygon(outline, fill=(STONE if b["kind"] == "church" else ROOF) + (255,), outline=INK + (255,), width=3)
        if b["kind"] == "church":
            x, y = P(c)
            d.line([(x - 9, y), (x + 9, y)], fill=INK, width=4); d.line([(x, y - 12), (x, y + 12)], fill=INK, width=4)
# The castle's keep inside the walls, drawn as a darker block (the curtain wall runs around it)
cx, cy = P(to_game(CENTRE))
k = 25 * PX / (2 * HALF) * 1.0
d.rectangle([cx - k * 1.0, cy - k * 1.3, cx + k * 1.0, cy + k * 1.3], fill=(130, 122, 110, 255), outline=INK + (255,), width=4)

# Points
for p in plan["points"]:
    x, y = P(p["position"])
    if p["kind"] == "earthwork":
        r = 28 * PX / (2 * HALF)
        d.ellipse([x - r, y - r, x + r, y + r], outline=(110, 90, 60, 255), width=10)
        d.ellipse([x - r * 0.55, y - r * 0.55, x + r * 0.55, y + r * 0.55], fill=(200, 196, 150, 255))
    elif p["kind"] == "cross":
        d.line([(x - 6, y), (x + 6, y)], fill=INK, width=4); d.line([(x, y - 9), (x, y + 9)], fill=INK, width=4)
    elif p["kind"] == "well":
        d.ellipse([x - 7, y - 7, x + 7, y + 7], fill=RIVER, outline=INK, width=2)
    elif p["kind"] == "mill":
        d.rectangle([x - 12, y - 9, x + 12, y + 9], fill=ROOF, outline=INK, width=3)
        d.ellipse([x + 10, y - 12, x + 26, y + 12], outline=INK, width=4)             # the wheel
    elif p["kind"] == "ford":
        for o in (-10, 0, 10):
            d.line([(x - 14, y + o), (x + 14, y + o)], fill=(240, 232, 205, 230), width=3)

# Labels (Cinzel, like the rest of the game), each with a paper-coloured halo
def text(p_game, s, size=30, face="Cinzel-SemiBold", colour=INK, anchor="mm"):
    x, y = P(p_game)
    d.text((x, y), s, font=font(face, size), fill=colour, anchor=anchor, stroke_width=4, stroke_fill=(247, 239, 212, 230))


text(to_game(CENTRE) + np.array([0, -70]), "Middleham Castle", 34, "Cinzel-Bold")
church = next(b for b in plan["buildings"] if b["kind"] == "church" and "Alkelda" in (b.get("name") or ""))
text(np.array(church["centre"]) + np.array([-10, 22]), "St Alkelda", 26)
text(to_game(np.array([65.0, 172.0])) + np.array([55, 12]), "Market Place", 24, "EBGaramond-SemiBold")
text(to_game(np.array([-83.0, 89.0])) + np.array([-70, 0]), "Swine Market", 22, "EBGaramond-SemiBold")
text(to_game(np.array([-208.0, -313.0])) + np.array([0, -45]), "William's Hill", 26)
for a in parks:
    c = np.mean(np.array(a["outline"]), axis=0)
    text(c + (np.array([60, -40]) if a["name"] == "Sunskew Park" else np.array([40, -10])), a["name"], 30, "Cinzel-Regular", (80, 70, 40))
spen = [b for b in plan["buildings"] if b.get("place") == "Spennithorne" or ("Michael" in (b.get("name") or ""))]
if spen:
    c = np.mean(np.array([b["centre"] for b in spen]), axis=0)
    text(c + np.array([0, -60]), "Spennithorne", 28)
text(np.array([-60, 330]), "MIDDLEHAM", 54, "Cinzel-Bold")
for r, pos in (("River Ure", np.array([140, 420])), ("River Cover", np.array([-120, -420]))):
    text(pos, r, 30, "EBGaramond-Italic", RIVER)
for p in plan["points"]:
    if p["kind"] in ("mill", "ford"):
        text(np.array(p["position"]) + np.array([0, -24]), p["name"], 20, "EBGaramond-SemiBold")
# Where the roads lead
text(np.array([470, 90]), "to Ripon", 22, "EBGaramond-Italic", ROAD, "rm")
text(np.array([-330, 470]), "to Leyburn & Hawes", 22, "EBGaramond-Italic", ROAD, "lm")
text(np.array([-470, -260]), "to Coverdale", 22, "EBGaramond-Italic", ROAD, "lm")

os.makedirs("Art/UI/Maps", exist_ok=True)
img.convert("RGB").save("Art/UI/Maps/localmap_middleham.png")

# The checking copy: margins, title, scale bar, north, legend
M = 150
sheet = Image.new("RGB", (PX + 2 * M, PX + 2 * M + 120), tuple(PARCH)); sheet.paste(img.convert("RGB"), (M, M + 60))
sd = ImageDraw.Draw(sheet)
sd.rectangle([M - 4, M + 56, M + PX + 4, M + PX + 64], outline=INK, width=4)
sd.text((M, 40), "Middleham, 1455  -  plan of the 1 x 1 km test landscape", font=font("Cinzel-Bold", 56), fill=INK)
sd.text((M, M + PX + 80), "Real place squeezed: about 1:1.4 in the town, 1:5 at the rivers (1:3 on average). Buildings at real size, fewer of them. "
        "100 m in the game = the bar below.", font=font("EBGaramond-Regular", 30), fill=INK)
bar = 100 * PX / (2 * HALF)
sd.line([(M, M + PX + 150), (M + bar, M + PX + 150)], fill=INK, width=8)
sd.text((M + bar + 20, M + PX + 132), "100 m", font=font("EBGaramond-SemiBold", 30), fill=INK)
sd.text((M + PX - 60, M + 70), "N", font=font("Cinzel-Bold", 60), fill=INK)
sheet.save("Docs/World/Middleham/middleham_1455_plan.png")
print("drawn")
