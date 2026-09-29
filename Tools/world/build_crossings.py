"""
Find every place a road, lane or track crosses a river and decide how it crosses: bridge (stone or timber), ford or
ferry. Documented medieval bridges near a crossing are named.
    python Tools/world/build_crossings.py
Reads the rivers (Rivers_England1455.json), main roads (Roads_England1455.json) and local roads
(LocalRoads_England1455.json); writes Data/World/Crossings_England1455.json.
Rules (river widths in the game, 1:3 of real):
  main and great roads: always a bridge; stone at a town or where the river is 12 m or wider, else timber
  lanes: ford under 6 m; timber bridge 6-15 m (stone at a town); wider: stone bridge at a town, else a ferry
  tracks: ford under 8 m; wider: timber footbridge near a settlement, else a ferry
"""
import csv
import json
import math
import sys

ROOT = "C:/Dev/CountriesIRL_3D_Game"
sys.path.insert(0, f"{ROOT}/Tools/world")
import draw_plan_map as m  # noqa: E402

AT_TOWN_KM = 2.5        # a crossing this close (real km) to a city or town is "at" it
MERGE_KM = 0.4          # crossings closer than this (same river) are one crossing

# Documented medieval bridges standing in 1455 (name -> the place it's at); crossings near these places take the name
KNOWN_BRIDGES = {   # name: (the place it's at, the river it crosses)
    "London Bridge": ("London", "Thames"), "Ouse Bridge, York": ("York", "Ouse"), "Ferrybridge (the Aire bridge)": ("Ferrybridge", "Aire"),
    "Wakefield Bridge and its chapel": ("Wakefield", "Calder"), "Tadcaster Bridge": ("Tadcaster", "Wharfe"),
    "Rochester Bridge": ("Rochester", "Medway"), "Culham Bridge (1416)": ("Abingdon", "Thames"),
    "Grandpont (Folly Bridge), Oxford": ("Oxford", "Thames"), "Kingston Bridge": ("Kingston upon Thames", "Thames"),
    "Windsor Bridge": ("Windsor", "Thames"), "Wallingford Bridge": ("Wallingford", "Thames"), "Maidenhead Bridge": ("Maidenhead", "Thames"),
    "Henley Bridge": ("Henley-on-Thames", "Thames"), "Trent Bridge (Hethbeth), Nottingham": ("Nottingham", "Trent"),
    "Newark Bridge": ("Newark", "Trent"), "Burton Bridge": ("Burton upon Trent", "Trent"), "Huntingdon Bridge": ("Huntingdon", "Great Ouse"),
    "Bedford Bridge": ("Bedford", "Great Ouse"), "Great Bridge, Cambridge": ("Cambridge", "Cam"), "Stamford Bridge (Welland)": ("Stamford", "Welland"),
    "Framwellgate and Elvet Bridges, Durham": ("Durham", "Wear"), "Tyne Bridge, Newcastle": ("Newcastle upon Tyne", "Tyne"),
    "Eden Bridges, Carlisle": ("Carlisle", "Eden"), "Lune Bridge, Lancaster": ("Lancaster", "Lune"), "Ribble Bridge, Preston": ("Preston", "Ribble"),
    "Warrington Bridge": ("Warrington", "Mersey"), "Welsh and English Bridges, Shrewsbury": ("Shrewsbury", "Severn"),
    "Bridgnorth Bridge": ("Bridgnorth", "Severn"), "Worcester Bridge": ("Worcester", "Severn"), "Westgate Bridge, Gloucester": ("Gloucester", "Severn"),
    "King John's Bridge, Tewkesbury": ("Tewkesbury", "Avon"), "Bristol Bridge": ("Bristol", "Avon"), "Bath Bridge": ("Bath", "Avon"),
    "Barnard Castle Bridge": ("Barnard Castle", "Tees"), "Boroughbridge": ("Boroughbridge", "Ure"), "Ripon Bridges": ("Ripon", "Ure"),
    "Ludford Bridge": ("Ludlow", "Teme"), "Yarm Bridge": ("Yarm", "Tees"), "Bow Bridge, Stratford-at-Bow": ("London", "Lea"),
}


def seg_intersect(p1, p2, q1, q2):
    """Crossing point of segments p1-p2 and q1-q2, or None."""
    d = (p2[0] - p1[0]) * (q2[1] - q1[1]) - (p2[1] - p1[1]) * (q2[0] - q1[0])
    if abs(d) < 1e-12:
        return None
    t = ((q1[0] - p1[0]) * (q2[1] - q1[1]) - (q1[1] - p1[1]) * (q2[0] - q1[0])) / d
    u = ((q1[0] - p1[0]) * (p2[1] - p1[1]) - (q1[1] - p1[1]) * (p2[0] - p1[0])) / d
    if 0.0 <= t <= 1.0 and 0.0 <= u <= 1.0:
        return (p1[0] + t * (p2[0] - p1[0]), p1[1] + t * (p2[1] - p1[1])), u
    return None


def main():
    units = json.load(open(f"{ROOT}/Art/MapData/ne_10m_admin_0_map_units.geojson", encoding="utf-8"))
    allp = {r["Id"]: r for r in csv.DictReader(open(f"{ROOT}/Data/World/Places_England1455.csv", encoding="utf-8"))}
    places = m.release_places(allp, units)
    waypoints = {r["Name"]: (float(r["Lat"]), float(r["Lon"])) for r in csv.DictReader(open(f"{ROOT}/Data/World/Road_Waypoints.csv", encoding="utf-8"))}
    P = lambda r: m.km(float(r["Lon"]), float(r["Lat"]))
    towns = [(r["Name"], P(r)) for r in places.values() if r["Type"] in ("City", "Town")]
    settlements = [(r["Name"], P(r)) for r in places.values() if r["Type"] in ("City", "Town", "Village")]
    named = {name: P(allp_row) for allp_row in allp.values() for name in [allp_row["Name"]]}
    named.update({n.split(",")[0]: m.km(lon, lat) for n, (lat, lon) in waypoints.items()})

    rivers = [r for r in json.load(open(f"{ROOT}/Data/World/Rivers_England1455.json", encoding="utf-8"))["rivers"]
              if not m.is_cut(*r["line"][-1])]
    roads = [(r["name"], r["class"], [m.km(*p) for p in m.road_in_release(r["line"])])
             for r in json.load(open(f"{ROOT}/Data/World/Roads_England1455.json", encoding="utf-8"))["roads"]]
    local = json.load(open(f"{ROOT}/Data/World/LocalRoads_England1455.json", encoding="utf-8"))["roads"]
    roads += [(f"{r['from']} - {r['to']}", r["class"], [m.km(*p) for p in r["line"]]) for r in local]

    found = []
    for rv in rivers:
        rline = [m.km(*p) for p in rv["line"]]
        run = [0.0]
        for a, b in zip(rline, rline[1:]):
            run.append(run[-1] + math.dist(a, b))
        total = run[-1] or 1.0
        for name, kind, line in roads:
            for a, b in zip(line, line[1:]):
                for i, (c, d) in enumerate(zip(rline, rline[1:])):
                    hit = seg_intersect(a, b, c, d)
                    if not hit:
                        continue
                    pt, u = hit
                    t = (run[i] + u * math.dist(c, d)) / total          # 0 at the source, 1 at the mouth
                    w0, w1 = rv["width_game_m"]
                    width = w0 + (w1 - w0) * math.sqrt(t)
                    nav = rv["navigable_from"] is not None and t > rv["navigable_from"]
                    found.append({"pt": pt, "river": rv["name"], "river_id": rv["id"], "road": name, "road_class": kind,
                                  "width": width, "navigable": nav})

    # One crossing per river spot: keep the most important road there (great > main > lane > track)
    rank = {"great": 0, "main": 1, "lane": 2, "track": 3}
    found.sort(key=lambda f: rank[f["road_class"]])
    crossings = []
    for f in found:
        if any(c["river_id"] == f["river_id"] and math.dist(c["pt"], f["pt"]) < MERGE_KM for c in crossings):
            continue
        crossings.append(f)

    # Each documented bridge goes to the one nearest crossing of its own river (within 3.5 km of its place)
    for bridge, (place, river) in KNOWN_BRIDGES.items():
        if place not in named:
            continue
        options = [c for c in crossings if c["river"] == river and "known" not in c and math.dist(named[place], c["pt"]) <= 3.5]
        if options:
            min(options, key=lambda c: math.dist(named[place], c["pt"]))["known"] = bridge

    out = []
    for c in crossings:
        town, town_d = min(((n, math.dist(p, c["pt"])) for n, p in towns), key=lambda x: x[1])
        near, near_d = min(((n, math.dist(p, c["pt"])) for n, p in settlements), key=lambda x: x[1])
        at_town = town_d <= AT_TOWN_KM
        w, k = c["width"], c["road_class"]
        if k in ("great", "main"):
            kind = "stone bridge" if (at_town or w >= 12) else "timber bridge"
        elif k == "lane":
            kind = "ford" if w < 6 else ("stone bridge" if at_town else "timber bridge") if w < 15 else ("stone bridge" if at_town else "ferry")
        else:
            kind = "ford" if w < 8 else ("timber footbridge" if near_d <= 3 else "ferry")
        known = c.get("known")
        if known and "bridge" not in kind:
            kind = "stone bridge"                      # a documented bridge stands here
        lon, lat = c["pt"][0] / m.KX + m.LON0, c["pt"][1] / m.KY + m.LAT0
        out.append({"kind": kind, "name": known or f"{c['river']} {kind} near {near}", "river": c["river"],
                    "road": c["road"], "road_class": k, "river_width_game_m": round(w, 1), "boats_pass": c["navigable"],
                    "near": near, "documented": bool(known), "lon": round(lon, 4), "lat": round(lat, 4)})
    json.dump({"note": "Where roads cross rivers (1455). Documented bridges are named; the rest follow the rules in "
                       "Tools/world/build_crossings.py.", "crossings": out},
              open(f"{ROOT}/Data/World/Crossings_England1455.json", "w", encoding="utf-8"), indent=1)
    from collections import Counter
    print(len(out), "crossings:", dict(Counter(o["kind"] for o in out)), "| documented bridges:", sum(o["documented"] for o in out),
          "| on boat routes:", sum(o["boats_pass"] for o in out))
    print("documented:", sorted({o["name"] for o in out if o["documented"]}))


if __name__ == "__main__":
    main()
