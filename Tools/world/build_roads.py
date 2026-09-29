"""
Build the game's main roads of 1455 from a list of the towns each road passes through.
    python Tools/world/build_roads.py
Reads Data/World/Roads_England1455.csv (Route = stops separated by ';': a place Id from Places_England1455.csv, or a
Wikipedia title for a town that is not a map place). Titles are looked up once on Wikipedia and kept in
Data/World/Road_Waypoints.csv, so later runs work offline. Writes Data/World/Roads_England1455.json: one smooth
line per road (lon/lat), its width in the game and its length.
"""
import csv
import json
import math
import os
import sys
import time
import urllib.parse
import urllib.request

ROOT = "C:/Dev/CountriesIRL_3D_Game"
WAYPOINTS = f"{ROOT}/Data/World/Road_Waypoints.csv"
UA = "CrownsAndCommoners-dev/0.1 (indie game prototype; map planning)"
# Width of the road surface in the game, metres. Roads are not compressed: only the land between places is.
WIDTH_M = {"great": 8.0,    # the great roads (Roman-built): room for carts to pass with riders beside them
           "main": 6.0}     # main roads: two carts can pass


def km(lat, lon):
    return lon * 111.32 * math.cos(math.radians(53.0)), lat * 110.57


def dist(a, b):
    (x1, y1), (x2, y2) = km(*a), km(*b)
    return math.hypot(x1 - x2, y1 - y2)


def wiki_coords(titles):
    q = urllib.parse.urlencode({"action": "query", "prop": "coordinates", "titles": "|".join(titles),
                                "redirects": 1, "format": "json", "colimit": "max"})
    req = urllib.request.Request("https://en.wikipedia.org/w/api.php?" + q, headers={"User-Agent": UA})
    data = json.load(urllib.request.urlopen(req, timeout=30))
    names = {t: t for t in titles}
    for r in data["query"].get("normalized", []) + data["query"].get("redirects", []):
        for k, v in list(names.items()):
            if v == r["from"]:
                names[k] = r["to"]
    found = {p["title"]: (p["coordinates"][0]["lat"], p["coordinates"][0]["lon"])
             for p in data["query"]["pages"].values() if "coordinates" in p}
    return {t: found.get(names[t]) for t in titles}


def through_stops(pts, steps=8):
    """A smooth curve that passes exactly through every stop (centripetal Catmull-Rom), in lat/lon."""
    P = [km(*p) for p in pts]
    P = [P[0]] + P + [P[-1]]
    out = [pts[0]]
    for i in range(1, len(P) - 2):
        p0, p1, p2, p3 = P[i - 1], P[i], P[i + 1], P[i + 2]
        t0 = 0.0
        t1 = t0 + max(math.dist(p0, p1), 1e-6) ** 0.5
        t2 = t1 + max(math.dist(p1, p2), 1e-6) ** 0.5
        t3 = t2 + max(math.dist(p2, p3), 1e-6) ** 0.5
        for k in range(1, steps + 1):
            t = t1 + (t2 - t1) * k / steps
            lerp = lambda a, b, ta, tb: tuple(a[j] * (tb - t) / (tb - ta) + b[j] * (t - ta) / (tb - ta) for j in (0, 1))
            a1, a2, a3 = lerp(p0, p1, t0, t1), lerp(p1, p2, t1, t2), lerp(p2, p3, t2, t3)
            b1, b2 = lerp(a1, a2, t0, t2), lerp(a2, a3, t1, t3)
            x, y = lerp(b1, b2, t1, t2)
            out.append((y / 110.57, x / (111.32 * math.cos(math.radians(53.0)))))
    return out


def main():
    places = {r["Id"]: r for r in csv.DictReader(open(f"{ROOT}/Data/World/Places_England1455.csv", encoding="utf-8"))}
    roads = list(csv.DictReader(open(f"{ROOT}/Data/World/Roads_England1455.csv", encoding="utf-8")))
    known = {}
    if os.path.exists(WAYPOINTS):
        known = {r["Name"]: (float(r["Lat"]), float(r["Lon"])) for r in csv.DictReader(open(WAYPOINTS, encoding="utf-8"))}
    titles = sorted({s for r in roads for s in r["Route"].split(";") if s not in places and s not in known})
    for i in range(0, len(titles), 45):
        known.update({t: c for t, c in wiki_coords(titles[i:i + 45]).items() if c})
        time.sleep(1)
    missing = [t for t in titles if t not in known]
    if missing:
        sys.exit(f"Not found on Wikipedia (fix the title in Roads_England1455.csv): {missing}")
    with open(WAYPOINTS, "w", encoding="utf-8", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["Name", "Lat", "Lon"])
        for name in sorted(known):
            w.writerow([name, f"{known[name][0]:.4f}", f"{known[name][1]:.4f}"])

    def where(stop):
        if stop in places:
            return float(places[stop]["Lat"]), float(places[stop]["Lon"])
        return known[stop]

    out, problems = [], []
    for r in roads:
        stops = r["Route"].split(";")
        pts = [where(s) for s in stops]
        for s, a, b in zip(stops[1:], pts, pts[1:]):
            if dist(a, b) > 60:                 # a wrong Wikipedia match shows up as a huge jump
                problems.append(f"{r['Id']}: {s} is {dist(a, b):.0f} km from the previous stop")
        line = through_stops(pts)
        length = sum(dist(a, b) for a, b in zip(line, line[1:]))
        out.append({"id": r["Id"], "name": r["Name"], "class": r["Class"], "width_m": WIDTH_M[r["Class"]],
                    "stops": [places[s]["Name"] if s in places else s.split(",")[0] for s in stops],
                    "length_km": round(length), "note": r["Note"],
                    "line": [[round(lon, 4), round(lat, 4)] for lat, lon in line]})
        print(f"{r['Id']:18s} {r['Class']:5s} {length:5.0f} km  {len(stops)} stops")
    if problems:
        sys.exit("Check these stops:\n" + "\n".join(problems))
    json.dump({"roads": out}, open(f"{ROOT}/Data/World/Roads_England1455.json", "w", encoding="utf-8"), indent=1)
    print(len(out), "roads ->", "Data/World/Roads_England1455.json")


if __name__ == "__main__":
    main()
