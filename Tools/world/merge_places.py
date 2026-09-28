"""
Build Data/World/Places_England1455.csv from its parts:
  - the hand-picked list itself (kept as is)
  - Data/World/Places_England1455_more.txt (Id|Name|Type|Lat|Lon|Importance|Note|Wikipedia title)
  - Art/MapData/wikidata_religious_houses.csv and wikidata_castles.csv (inside England only; places already on the
    list within 1.5 km are skipped)
    python Tools/world/merge_places.py
"""
import csv
import json
import math
import os
import re

PATH = "Data/World/Places_England1455.csv"
FIELDS = ["Id", "Name", "Type", "Lat", "Lon", "Importance", "Note", "Wiki"]


def slug(name):
    return re.sub(r"[^a-z0-9]+", "_", name.lower()).strip("_")


def england_polygons():
    units = json.load(open("Art/MapData/ne_10m_admin_0_map_units.geojson", encoding="utf-8"))
    for f in units["features"]:
        if f["properties"].get("NAME") == "England":
            g = f["geometry"]
            return [p[0] for p in (g["coordinates"] if g["type"] == "MultiPolygon" else [g["coordinates"]])]
    return []


def inside(lon, lat, polys):
    for ring in polys:
        hit = False
        for i in range(len(ring)):
            (x1, y1), (x2, y2) = ring[i - 1], ring[i]
            if (y1 > lat) != (y2 > lat) and lon < (x2 - x1) * (lat - y1) / (y2 - y1) + x1:
                hit = not hit
        if hit:
            return True
    return False


def dist_km(a, b):
    return math.hypot((a[0] - b[0]) * 110.57, (a[1] - b[1]) * 111.32 * math.cos(math.radians(a[0])))


rows = list(csv.DictReader(open(PATH, encoding="utf-8")))
for r in rows:
    r.setdefault("Wiki", "")
ids = {r["Id"] for r in rows}


def add(row):
    if row["Id"] in ids:
        return False
    ids.add(row["Id"])
    rows.append(row)
    return True


added = 0
for line in open("Data/World/Places_England1455_more.txt", encoding="utf-8"):
    line = line.strip()
    if not line or line.startswith("#"):
        continue
    i, n, t, la, lo, imp, note, wiki = line.split("|")
    added += add(dict(Id=i, Name=n, Type=t, Lat=la, Lon=lo, Importance=imp, Note=note, Wiki=wiki))

polys = england_polygons()


def near_existing(lat, lon, types):
    return any(r["Type"] in types and dist_km((lat, lon), (float(r["Lat"]), float(r["Lon"]))) < 1.5 for r in rows)


auto = {"religious": 0, "castles": 0}
path = "Art/MapData/wikidata_religious_houses.csv"
if os.path.exists(path):
    for r in csv.DictReader(open(path, encoding="utf-8")):
        lat, lon = float(r["Lat"]), float(r["Lon"])
        if r["Name"].startswith("Q") or not inside(lon, lat, polys) or near_existing(lat, lon, ("Abbey", "Cathedral")):
            continue
        auto["religious"] += add(dict(Id="rh_" + slug(r["Name"]), Name=r["Name"], Type="Abbey", Lat=f"{lat:.4f}", Lon=f"{lon:.4f}",
                                      Importance="1", Note=f"{r['Kind']}; dissolved {r['Dissolved']} (Wikidata)", Wiki=""))
path = "Art/MapData/wikidata_castles.csv"
if os.path.exists(path):
    for r in csv.DictReader(open(path, encoding="utf-8")):
        lat, lon = float(r["Lat"]), float(r["Lon"])
        if r["Name"].startswith("Q") or not inside(lon, lat, polys) or near_existing(lat, lon, ("Castle",)):
            continue
        auto["castles"] += add(dict(Id="c_" + slug(r["Name"]), Name=r["Name"], Type="Castle", Lat=f"{lat:.4f}", Lon=f"{lon:.4f}",
                                    Importance="1", Note=f"Built {r['Built']} (Wikidata)", Wiki=""))

w = csv.DictWriter(open(PATH, "w", encoding="utf-8", newline=""), fieldnames=FIELDS)
w.writeheader()
w.writerows(rows)
counts = {}
for r in rows:
    counts[r["Type"]] = counts.get(r["Type"], 0) + 1
print(f"{len(rows)} places (+{added} hand-picked, +{auto['religious']} religious houses, +{auto['castles']} castles)")
print(counts)
