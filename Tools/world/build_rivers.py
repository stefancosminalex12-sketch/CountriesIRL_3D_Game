"""
Build the game's simplified, connected river network from OpenStreetMap river pieces.
    python Tools/world/build_rivers.py
Reads the hand-picked list Data/World/Rivers_England1455.csv (which rivers, a point on each, what it flows into,
how far boats could go in 1455, Border=1 if it forms a land border, StartNear=a place Id to begin the river there) and Art/MapData/osm_rivers_england.geojson (fetch_map_data.py).
Writes Data/World/Rivers_England1455.json: for every river one smooth centreline from source to mouth (lon/lat),
ending exactly on the river it flows into, plus how much of it is navigable.
"""
import csv
import heapq
import json
import math
from collections import defaultdict

ROOT = "C:/Dev/CountriesIRL_3D_Game"
LAT0, LON0 = 53.0, -1.9
KX = 111.32 * math.cos(math.radians(LAT0))
KY = 110.57
COMPRESSION = 12.0         # distances in the game (1:12)
WIDTH_SCALE = 3.0          # river widths in the game (1:3, like towns)
# Real width of the river (m) at its source and at its mouth, by size: 1 great, 2 main, 3 smaller
REAL_WIDTH_M = {1: (8, 150), 2: (5, 60), 3: (3, 25)}
BEND_WIDTHS = 4            # a river cannot bend tighter than ~4 of its own widths, so smaller bends are smoothed
MIN_SIMPLIFY_KM = 1.0      # never keep bends smaller than this (real km; ~80 m in the game)
GAP_KM = 6.0               # pieces of the same river closer than this are joined (OSM misses some stretches)


def km(lon, lat):
    return (lon - LON0) * KX, (lat - LAT0) * KY


def lonlat(x, y):
    return x / KX + LON0, y / KY + LAT0


def dist(a, b):
    return math.hypot(a[0] - b[0], a[1] - b[1])


def nearest_on_line(p, line):
    """Closest point to p on a polyline: (distance, point, segment index)."""
    best = (1e18, None, 0)
    for i in range(len(line) - 1):
        (x1, y1), (x2, y2) = line[i], line[i + 1]
        dx, dy = x2 - x1, y2 - y1
        t = 0.0 if dx == dy == 0 else max(0.0, min(1.0, ((p[0] - x1) * dx + (p[1] - y1) * dy) / (dx * dx + dy * dy)))
        q = (x1 + t * dx, y1 + t * dy)
        dd = dist(p, q)
        if dd < best[0]:
            best = (dd, q, i)
    return best


def douglas_peucker(pts, tol):
    if len(pts) < 3:
        return pts
    a, b = pts[0], pts[-1]
    norm = dist(a, b) or 1e-12
    far, idx = 0.0, 0
    for i in range(1, len(pts) - 1):
        d = abs((b[1] - a[1]) * pts[i][0] - (b[0] - a[0]) * pts[i][1] + b[0] * a[1] - b[1] * a[0]) / norm
        if d > far:
            far, idx = d, i
    if far <= tol:
        return [a, b]
    return douglas_peucker(pts[:idx + 1], tol)[:-1] + douglas_peucker(pts[idx:], tol)


def chaikin(pts, rounds=2):
    """Rounds the corners off, keeping both ends fixed."""
    for _ in range(rounds):
        out = [pts[0]]
        for a, b in zip(pts, pts[1:]):
            out += [(0.75 * a[0] + 0.25 * b[0], 0.75 * a[1] + 0.25 * b[1]), (0.25 * a[0] + 0.75 * b[0], 0.25 * a[1] + 0.75 * b[1])]
        out.append(pts[-1])
        pts = out
    return pts


def main_stem(pieces, seed, target):
    """The river's main channel as one line, from the source down to the end nearest `target(point) -> km`."""
    key = lambda p: (round(p[0], 2), round(p[1], 2))          # ~10 m
    nodes, adj = {}, defaultdict(dict)                     # adj[k][k2] = length; a piece listed twice counts once
    for line in pieces:
        for a, b in zip(line, line[1:]):
            ka, kb = key(a), key(b)
            if ka == kb:
                continue
            nodes[ka], nodes[kb] = a, b
            adj[ka][kb] = adj[kb][ka] = dist(a, b)
    # Bridge small gaps between separate pieces of the river (lakes, culverts, missing bits): one link, at the
    # closest pair of loose ends, per pair of pieces. Never inside one piece, or side channels become shortcuts.
    comp = {}
    for k0 in adj:
        if k0 in comp:
            continue
        comp[k0], stack = k0, [k0]
        while stack:
            for k2 in adj[stack.pop()]:
                if k2 not in comp:
                    comp[k2] = k0
                    stack.append(k2)
    ends = [k for k in adj if len(adj[k]) == 1]
    links = {}
    for i, k in enumerate(ends):
        for k2 in ends[i + 1:]:
            pair = tuple(sorted((comp[k], comp[k2])))
            d = dist(nodes[k], nodes[k2])
            if pair[0] != pair[1] and d < GAP_KM and d < links.get(pair, (1e18,))[0]:
                links[pair] = (d, k, k2)
    for d, k, k2 in links.values():
        adj[k][k2] = adj[k2][k] = d

    def reach(start):
        best = {start: 0.0}
        prev = {}
        heap = [(0.0, start)]
        while heap:
            d, k = heapq.heappop(heap)
            if d > best[k]:
                continue
            for k2, w in adj[k].items():
                if d + w < best.get(k2, 1e18):
                    best[k2], prev[k2] = d + w, k
                    heapq.heappush(heap, (d + w, k2))
        return best, prev

    start = min(nodes, key=lambda k: dist(nodes[k], seed))
    part, _ = reach(start)                                     # the piece of river network the seed is on
    mouth = min(part, key=lambda k: target(nodes[k]))
    best, prev = reach(mouth)
    # Source: far from the mouth both along the river and as the crow flies (a loose side channel reached only
    # from its top end is far along the river but close to the mouth)
    source = max(best, key=lambda k: best[k] * dist(nodes[k], nodes[mouth]))
    path = [source]
    while path[-1] != mouth:
        path.append(prev[path[-1]])
    return [nodes[k] for k in path], dist(nodes[start], seed)


def main():
    osm = json.load(open(f"{ROOT}/Art/MapData/osm_rivers_england.geojson", encoding="utf-8"))
    by_name = defaultdict(list)
    for f in osm["features"]:
        by_name[f["properties"]["name"]].append([km(*c) for c in f["geometry"]["coordinates"]])
    def pieces_of(names):          # "Great Stour|River Stour"; OSM names some stretches "Ouse" instead of "River Ouse"
        spellings = {v for n in names.split("|") for v in (n, n.removeprefix("River "), "River " + n.removeprefix("River "))}
        return [p for n in spellings for p in by_name[n]]

    units = json.load(open(f"{ROOT}/Art/MapData/ne_10m_admin_0_map_units.geojson", encoding="utf-8"))
    # Coastline = outline points of England, Wales and Scotland that are not on a land border between them
    owners = defaultdict(set)
    for f in units["features"]:
        name = f["properties"].get("NAME")
        if name in ("England", "Wales", "Scotland"):
            g = f["geometry"]
            for poly in (g["coordinates"] if g["type"] == "MultiPolygon" else [g["coordinates"]]):
                for c in poly[0]:
                    owners[(round(c[0], 5), round(c[1], 5))].add(name)
    coast = [km(*c) for c, o in owners.items() if len(o) == 1 and 49.8 < c[1] < 56.0 and c[0] > -6.0]

    places = {r["Id"]: r for r in csv.DictReader(open(f"{ROOT}/Data/World/Places_England1455.csv", encoding="utf-8"))}
    defs = list(csv.DictReader(open(f"{ROOT}/Data/World/Rivers_England1455.csv", encoding="utf-8")))
    built = {}
    # Rivers into the sea first, then each tributary after the river it joins
    pending = list(defs)
    while pending:
        progressed = False
        for r in list(pending):
            parent = r["FlowsInto"]
            if parent != "sea" and parent not in built:
                continue
            pending.remove(r)
            progressed = True
            seed = km(float(r["SeedLon"]), float(r["SeedLat"]))
            if parent == "sea":
                target = lambda p: min(dist(p, c) for c in coast)
            else:
                pline = built[parent]["km"]
                target = lambda p, pl=pline: nearest_on_line(p, pl)[0]
            stem, seed_off = main_stem(pieces_of(r["OsmName"]), seed, target)
            if parent != "sea":                                # end exactly on the river it joins
                stem.append(nearest_on_line(stem[-1], built[parent]["km"])[1])
            mouth_game_m = REAL_WIDTH_M[int(r["Tier"])][1] / WIDTH_SCALE
            tolerance = max(MIN_SIMPLIFY_KM, BEND_WIDTHS * mouth_game_m * COMPRESSION / 1000)
            smooth = chaikin(douglas_peucker(stem, tolerance))
            if r.get("StartNear"):                             # drawn only from this place down (Severn: not from Wales)
                pl = places[r["StartNear"]]
                start = km(float(pl["Lon"]), float(pl["Lat"]))
                smooth = smooth[min(range(len(smooth)), key=lambda k: dist(smooth[k], start)):]
            length = sum(dist(a, b) for a, b in zip(smooth, smooth[1:]))
            nav_from = None                                    # fraction along the line (source=0, mouth=1)
            if r["NavigableTo"]:
                pl = places.get(r["NavigableTo"])
                head = km(float(r["NavLon"] or pl["Lon"]), float(r["NavLat"] or pl["Lat"]))
                _, _, seg = nearest_on_line(head, smooth)
                nav_from = sum(dist(a, b) for a, b in zip(smooth[:seg + 1], smooth[1:seg + 1])) / length
            built[r["Id"]] = {"km": smooth, "row": r, "length": length, "nav_from": nav_from}
            flag = "  <-- check: seed far from the river" if seed_off > 3 else ""
            print(f"{r['Id']:15s} {length:6.0f} km  {len(smooth):4d} pts  seed {seed_off:4.1f} km off{flag}")
        if not progressed:
            raise SystemExit(f"Unknown FlowsInto: {[r['Id'] + '->' + r['FlowsInto'] for r in pending]}")

    out = []
    for r in defs:
        b = built[r["Id"]]
        out.append({"id": r["Id"], "name": r["Name"], "flows_into": r["FlowsInto"], "tier": int(r["Tier"]),
                    "navigable_to": r["NavigableTo"] or None,
                    "navigable_from": None if b["nav_from"] is None else round(b["nav_from"], 3),
                    "length_km": round(b["length"]), "note": r["Note"], "border": r.get("Border") == "1",
                    "width_game_m": [round(w / WIDTH_SCALE, 1) for w in REAL_WIDTH_M[int(r["Tier"])]],
                    "line": [[round(v, 4) for v in lonlat(*p)] for p in b["km"]]})
    json.dump({"source": "(c) OpenStreetMap contributors (ODbL), simplified", "rivers": out},
              open(f"{ROOT}/Data/World/Rivers_England1455.json", "w", encoding="utf-8"), indent=1)
    print(len(out), "rivers ->", "Data/World/Rivers_England1455.json")


if __name__ == "__main__":
    main()
