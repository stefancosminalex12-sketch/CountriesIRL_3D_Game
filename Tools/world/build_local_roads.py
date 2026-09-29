"""
Build the local road network of the first release: lanes between towns and villages, tracks out to castles, abbeys
and landmarks, so every place can be reached. The main roads (build_roads.py) stay the backbone.
    python Tools/world/build_local_roads.py
Which places connect: each town/village links to its natural neighbours (Gabriel graph: no other settlement closer to
both), skipping links a main road already serves; places left unconnected get a spur to the nearest road. Sites
away from a settlement (castles, abbeys, landmarks) get a track to the nearest road or settlement.
Where they run: each link follows the game terrain (Data/World/Terrain_England1455.png) along the easiest line,
round steep ground and along valleys, like real lanes. Realistic, not documented: refine per region from old maps.
Writes Data/World/LocalRoads_England1455.json.
"""
import csv
import heapq
import json
import math
import sys

import numpy as np
from PIL import Image, ImageDraw

ROOT = "C:/Dev/CountriesIRL_3D_Game"
sys.path.insert(0, f"{ROOT}/Tools/world")
import draw_plan_map as m  # noqa: E402

WIDTH_M = {"lane": 4.0,     # one cart, room to pass at gateways
           "track": 2.5}    # to a castle, abbey or landmark
MAX_LANE_KM = 45.0          # real km: longer neighbour links are left to the main roads
NEAR_ROAD_KM = 1.5          # a place this close to a road is on it
SITE_IN_TOWN_KM = 1.0       # a castle/abbey this close to a settlement is part of it (no track)
GRID_KM = 0.5               # pathfinding cell (real km)
SLOPE_COST = 40.0           # how much steep ground is avoided: cost x (1 + SLOPE_COST * slope^2)

settle_types = ("City", "Town", "Village")
site_types = ("Castle", "Abbey", "Landmark", "Cathedral")


def load():
    units = json.load(open(f"{ROOT}/Art/MapData/ne_10m_admin_0_map_units.geojson", encoding="utf-8"))
    allp = {r["Id"]: r for r in csv.DictReader(open(f"{ROOT}/Data/World/Places_England1455.csv", encoding="utf-8"))}
    places = m.release_places(allp, units)
    roads = json.load(open(f"{ROOT}/Data/World/Roads_England1455.json", encoding="utf-8"))["roads"]
    road_lines = [[m.km(*p) for p in m.road_in_release(r["line"])] for r in roads]
    england = [f for f in units["features"] if f["properties"].get("NAME") == "England"][0]
    coast = m.game_coast([p[0] for p in m.geo_polys(england["geometry"])])
    return places, road_lines, coast


def seg_dist(p, a, b):
    dx, dy = b[0] - a[0], b[1] - a[1]
    t = 0.0 if dx == dy == 0 else max(0.0, min(1.0, ((p[0] - a[0]) * dx + (p[1] - a[1]) * dy) / (dx * dx + dy * dy)))
    q = (a[0] + t * dx, a[1] + t * dy)
    return math.dist(p, q), q


def nearest_on_lines(p, lines):
    best = (1e18, None, -1, 0.0)
    for li, line in enumerate(lines):
        run = 0.0
        for a, b in zip(line, line[1:]):
            d, q = seg_dist(p, a, b)
            if d < best[0]:
                best = (d, q, li, run + math.dist(a, q))
            run += math.dist(a, b)
    return best          # distance, point, line index, distance along that line


class Terrain:
    """Game heights on a coarse grid (GRID_KM real), with sea blocked; finds the easiest path between two points."""

    def __init__(self, coast):
        meta = json.load(open(f"{ROOT}/Data/World/Terrain_England1455.json"))
        h = np.asarray(Image.open(f"{ROOT}/Data/World/Terrain_England1455.png"), dtype=np.float32) / meta["value_per_game_m"]
        step = int(round(GRID_KM / meta["cell_real_km"]))
        self.h = h[::step, ::step]
        lon0, lat0, lon1, lat1 = meta["box_lonlat"]
        self.x0, self.y0 = m.km(lon0, lat0)
        self.x1, self.y1 = m.km(lon1, lat1)
        self.cell_game_m = GRID_KM * 1000 / m.COMPRESSION
        mask = Image.new("L", (self.h.shape[1], self.h.shape[0]), 0)
        dr = ImageDraw.Draw(mask)
        for ring in coast:
            dr.polygon([self.to_cell(*m.km(lon, lat))[::-1] for lon, lat in ring], fill=255)
        self.land = np.asarray(mask) > 0

    def to_cell(self, x, y):
        return (int(round((self.y1 - y) / GRID_KM)), int(round((x - self.x0) / GRID_KM)))   # row, col

    def to_km(self, r, c):
        return (self.x0 + c * GRID_KM, self.y1 - r * GRID_KM)

    def path(self, a, b):
        """Easiest route from a to b (real km points) as a list of real km points."""
        (r0, c0), (r1, c1) = self.to_cell(*a), self.to_cell(*b)
        pad = max(6, int(0.4 * max(abs(r1 - r0), abs(c1 - c0))))
        rmin, rmax = max(0, min(r0, r1) - pad), min(self.h.shape[0] - 1, max(r0, r1) + pad)
        cmin, cmax = max(0, min(c0, c1) - pad), min(self.h.shape[1] - 1, max(c0, c1) + pad)
        start, goal = (r0, c0), (r1, c1)
        best, prev = {start: 0.0}, {}
        heap = [(0.0, 0.0, start)]
        moves = [(-1, 0), (1, 0), (0, -1), (0, 1), (-1, -1), (-1, 1), (1, -1), (1, 1)]
        while heap:
            _, g, cur = heapq.heappop(heap)
            if cur == goal:
                break
            if g > best.get(cur, 1e18):
                continue
            r, c = cur
            for dr, dc in moves:
                nr, nc = r + dr, c + dc
                if not (rmin <= nr <= rmax and cmin <= nc <= cmax):
                    continue
                if not self.land[nr, nc] and (nr, nc) != goal:
                    continue
                run = math.hypot(dr, dc)
                slope = abs(float(self.h[nr, nc] - self.h[r, c])) / (run * self.cell_game_m)
                ng = g + run * (1.0 + SLOPE_COST * slope * slope)
                if ng < best.get((nr, nc), 1e18):
                    best[(nr, nc)] = ng
                    prev[(nr, nc)] = cur
                    heapq.heappush(heap, (ng + math.hypot(nr - r1, nc - c1), ng, (nr, nc)))
        if goal not in prev and goal != start:
            return [a, b]                                      # no land route found: straight line
        cells = [goal]
        while cells[-1] != start:
            cells.append(prev[cells[-1]])
        pts = [self.to_km(r, c) for r, c in reversed(cells)]
        pts[0], pts[-1] = a, b
        return pts


def smooth(pts):
    pts = m.douglas_peucker(pts, 0.35)
    for _ in range(2):
        out = [pts[0]]
        for a, b in zip(pts, pts[1:]):
            out += [(0.75 * a[0] + 0.25 * b[0], 0.75 * a[1] + 0.25 * b[1]), (0.25 * a[0] + 0.75 * b[0], 0.25 * a[1] + 0.75 * b[1])]
        out.append(pts[-1])
        pts = out
    return pts


def main():
    places, road_lines, coast = load()
    terrain = Terrain(coast)
    P = lambda r: m.km(float(r["Lon"]), float(r["Lat"]))
    settlements = {i: r for i, r in places.items() if r["Type"] in settle_types}
    sites = {i: r for i, r in places.items() if r["Type"] in site_types and r.get("CastleType") != "town"}
    ids = list(settlements)
    pos = {i: P(settlements[i]) for i in ids}

    # 1. Natural neighbours (Gabriel graph) between settlements
    links = []
    for a_i, a in enumerate(ids):
        for b in ids[a_i + 1:]:
            pa, pb = pos[a], pos[b]
            d = math.dist(pa, pb)
            if d > MAX_LANE_KM:
                continue
            mid, rad2 = ((pa[0] + pb[0]) / 2, (pa[1] + pb[1]) / 2), (d / 2) ** 2
            if any(math.dist(pos[c], mid) ** 2 < rad2 for c in ids if c not in (a, b)):
                continue
            # A main road already serving both (each near the same road, road not much longer): no lane
            da, qa, la, sa = nearest_on_lines(pa, road_lines)
            db, qb, lb, sb = nearest_on_lines(pb, road_lines)
            if da < NEAR_ROAD_KM and db < NEAR_ROAD_KM and la == lb and abs(sa - sb) < 1.35 * d:
                continue
            links.append((a, b, "lane"))

    # 2. Places left unconnected (no lane, not on a main road): a spur to the nearest road
    linked = {x for a, b, _ in links for x in (a, b)}
    spurs = []
    for i in ids:
        d, q, _, _ = nearest_on_lines(pos[i], road_lines)
        if i not in linked and d >= NEAR_ROAD_KM:
            spurs.append((i, q, "lane"))

    # 3. Tracks to sites away from any settlement
    tracks = []
    for i, r in sites.items():
        p = P(r)
        near_settle = min(math.dist(p, pos[s]) for s in ids)
        if near_settle <= SITE_IN_TOWN_KM:
            continue
        d_road, q, _, _ = nearest_on_lines(p, road_lines)
        s_best = min(ids, key=lambda s: math.dist(p, pos[s]))
        target = q if d_road < near_settle else pos[s_best]
        if min(d_road, near_settle) >= 0.3:
            tracks.append((i, target, "track"))

    out = []
    to_ll = lambda x, y: [round(x / m.KX + m.LON0, 4), round(y / m.KY + m.LAT0, 4)]
    def add(kind, frm, to, a, b):
        line = smooth(terrain.path(a, b))
        length = sum(math.dist(p, q) for p, q in zip(line, line[1:]))
        out.append({"class": kind, "width_m": WIDTH_M[kind], "from": frm, "to": to,
                    "length_km_real": round(length, 1), "line": [to_ll(*p) for p in line]})
    for a, b, kind in links:
        add(kind, settlements[a]["Name"], settlements[b]["Name"], pos[a], pos[b])
    for i, q, kind in spurs:
        add(kind, settlements[i]["Name"], "main road", pos[i], q)
    for i, q, kind in tracks:
        add(kind, sites[i]["Name"], "road", P(sites[i]), q)
    json.dump({"note": "Generated local roads (realistic, not documented): lanes between towns and villages, tracks to "
                       "sites. Refine per region from old maps.", "roads": out},
              open(f"{ROOT}/Data/World/LocalRoads_England1455.json", "w", encoding="utf-8"))
    lanes = [r for r in out if r["class"] == "lane"]
    trk = [r for r in out if r["class"] == "track"]
    print(f"{len(lanes)} lanes ({len(links)} links, {len(spurs)} spurs), {len(trk)} tracks; "
          f"lanes {sum(r['length_km_real'] for r in lanes) / m.COMPRESSION:.0f} game km, "
          f"tracks {sum(r['length_km_real'] for r in trk) / m.COMPRESSION:.0f} game km")


if __name__ == "__main__":
    main()
