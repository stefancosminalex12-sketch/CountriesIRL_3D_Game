"""
Download the extra data for the planning map into Art/MapData (cached; delete a file to fetch it again):
  - religious houses that existed in 1455 (Wikidata: monasteries dissolved 1500-1560 = Henry VIII's dissolution)
  - castles built before 1455 (Wikidata)
  - England's main rivers (OpenStreetMap via Overpass, simplified)
  - elevation (AWS Terrain Tiles, terrarium format) for hill shading
    python Tools/world/fetch_map_data.py
Sources: Wikidata (CC0), OpenStreetMap (ODbL, credit "(c) OpenStreetMap contributors"), AWS Terrain Tiles (SRTM etc.).
"""
import csv
import io
import json
import math
import os
import time
import urllib.parse
import urllib.request

from PIL import Image

OUT = "Art/MapData"
UA = "CrownsAndCommoners-dev/0.1 (indie game prototype; map planning)"
BBOX = (-6.0, 49.8, 2.0, 56.0)   # lon/lat


def get(url, data=None, tries=5):
    req = urllib.request.Request(url, data=data, headers={"User-Agent": UA, "Accept": "application/json"})
    for attempt in range(tries):
        try:
            with urllib.request.urlopen(req, timeout=180) as r:
                return r.read()
        except Exception as e:
            last = e
            time.sleep(65)
    raise last


def sparql(query):
    url = "https://query.wikidata.org/sparql?" + urllib.parse.urlencode({"query": query, "format": "json"})
    return json.loads(get(url))["results"]["bindings"]


def parse_point(wkt):
    lon, lat = wkt.replace("Point(", "").replace(")", "").split()
    return float(lat), float(lon)


BOX_SERVICE = f"""
  SERVICE wikibase:box {{
    ?item wdt:P625 ?coord .
    bd:serviceParam wikibase:cornerSouthWest "Point({BBOX[0]} {BBOX[1]})"^^geo:wktLiteral .
    bd:serviceParam wikibase:cornerNorthEast "Point({BBOX[2]} {BBOX[3]})"^^geo:wktLiteral .
  }}"""


def fetch_religious():
    path = f"{OUT}/wikidata_religious_houses.csv"
    if os.path.exists(path):
        return
    q = f"""SELECT DISTINCT ?item ?itemLabel ?coord ?end ?kindLabel WHERE {{
  {BOX_SERVICE}
  ?item wdt:P31 ?kind . ?kind wdt:P279* wd:Q44613 .
  ?item wdt:P576 ?end . FILTER(YEAR(?end) >= 1500 && YEAR(?end) <= 1560)
  SERVICE wikibase:label {{ bd:serviceParam wikibase:language "en". }}
}}"""
    rows = sparql(q)
    with open(path, "w", encoding="utf-8", newline="") as f:
        w = csv.writer(f)
        w.writerow(["Name", "Kind", "Lat", "Lon", "Dissolved", "Wikidata"])
        seen = set()
        for r in rows:
            if r["item"]["value"] in seen:
                continue
            seen.add(r["item"]["value"])
            lat, lon = parse_point(r["coord"]["value"])
            w.writerow([r["itemLabel"]["value"], r.get("kindLabel", {}).get("value", ""), lat, lon, r["end"]["value"][:4], r["item"]["value"]])
    print("religious houses:", len(seen))


def fetch_castles():
    path = f"{OUT}/wikidata_castles.csv"
    if os.path.exists(path):
        return
    q = f"""SELECT DISTINCT ?item ?itemLabel ?coord ?start WHERE {{
  {BOX_SERVICE}
  ?item wdt:P31 ?kind . ?kind wdt:P279* wd:Q23413 .
  ?item wdt:P571 ?start . FILTER(YEAR(?start) < 1455)
  SERVICE wikibase:label {{ bd:serviceParam wikibase:language "en". }}
}}"""
    rows = sparql(q)
    with open(path, "w", encoding="utf-8", newline="") as f:
        w = csv.writer(f)
        w.writerow(["Name", "Lat", "Lon", "Built", "Wikidata"])
        seen = set()
        for r in rows:
            if r["item"]["value"] in seen:
                continue
            seen.add(r["item"]["value"])
            lat, lon = parse_point(r["coord"]["value"])
            w.writerow([r["itemLabel"]["value"], lat, lon, r["start"]["value"][:4], r["item"]["value"]])
    print("castles:", len(seen))


RIVERS = ["Thames", "Severn", "Trent", "Great Ouse", "Tyne", "North Tyne", "South Tyne", "Tees", "Wear", "Swale", "Ure",
          "Nidd", "Wharfe", "Aire", "Calder", "Don", "Derwent", "Hull", "Mersey", "Dee", "Ribble", "Lune", "Eden",
          "Tweed", "Avon", "Wye", "Medway", "Stour", "Test", "Itchen", "Exe", "Tamar", "Dart", "Parrett", "Kennet",
          "Cherwell", "Nene", "Welland", "Witham", "Soar", "Tame", "Dove", "Wensum", "Yare", "Cam", "Lea", "Colne",
          "Wey", "Frome", "Teme", "Lugg", "Weaver", "Coquet", "Ouse", "Idle", "Ancholme", "Waveney", "Bure", "Orwell",
          "Blackwater", "Arun", "Adur", "Rother", "Axe", "Taw", "Torridge", "Fowey", "Camel", "Brue", "Windrush",
          "Evenlode", "Thame", "Ock", "Loddon", "Irwell", "Ribble", "Wyre", "Kent", "Leven", "Esk", "Aln", "Wansbeck",
          "Rye", "Foss", "Hodder", "Skell", "Cover", "Bain", "Nene", "Ise", "Tove", "Ouzel", "Stour"]


def douglas_peucker(points, tol):
    if len(points) < 3:
        return points
    (x1, y1), (x2, y2) = points[0], points[-1]
    dx, dy = x2 - x1, y2 - y1
    norm = math.hypot(dx, dy) or 1e-12
    far, index = 0.0, 0
    for i in range(1, len(points) - 1):
        x, y = points[i]
        d = abs(dy * x - dx * y + x2 * y1 - y2 * x1) / norm
        if d > far:
            far, index = d, i
    if far <= tol:
        return [points[0], points[-1]]
    return douglas_peucker(points[:index + 1], tol)[:-1] + douglas_peucker(points[index:], tol)


def fetch_rivers():
    path = f"{OUT}/osm_rivers_england.geojson"
    if os.path.exists(path):
        return
    names = "|".join(sorted(set(RIVERS)))
    q = f"""[out:json][timeout:170];
way["waterway"="river"]["name"~"^River ({names})$"]({BBOX[1]},{BBOX[0]},{BBOX[3]},{BBOX[2]});
out geom;"""
    data = json.loads(get("https://overpass-api.de/api/interpreter", data=urllib.parse.urlencode({"data": q}).encode()))
    features = []
    for el in data["elements"]:
        pts = [(g["lon"], g["lat"]) for g in el.get("geometry", [])]
        pts = douglas_peucker(pts, 0.002)   # ~150 m
        if len(pts) >= 2:
            features.append({"type": "Feature", "properties": {"name": el.get("tags", {}).get("name", "")},
                             "geometry": {"type": "LineString", "coordinates": [[round(x, 4), round(y, 4)] for x, y in pts]}})
    json.dump({"type": "FeatureCollection", "features": features}, open(path, "w", encoding="utf-8"))
    print("river pieces:", len(features))


def fetch_elevation(z=8):
    """Terrarium tiles covering the box, stitched into one image with its lon/lat bounds (stored in a .json)."""
    path = f"{OUT}/elevation_z{z}.png"
    if os.path.exists(path):
        return
    n = 2 ** z
    tx = lambda lon: int((lon + 180) / 360 * n)
    ty = lambda lat: int((1 - math.log(math.tan(math.radians(lat)) + 1 / math.cos(math.radians(lat))) / math.pi) / 2 * n)
    x0, x1 = tx(BBOX[0]), tx(BBOX[2])
    y0, y1 = ty(BBOX[3]), ty(BBOX[1])
    mosaic = Image.new("RGB", ((x1 - x0 + 1) * 256, (y1 - y0 + 1) * 256))
    for x in range(x0, x1 + 1):
        for y in range(y0, y1 + 1):
            raw = get(f"https://s3.amazonaws.com/elevation-tiles-prod/terrarium/{z}/{x}/{y}.png")
            mosaic.paste(Image.open(io.BytesIO(raw)).convert("RGB"), ((x - x0) * 256, (y - y0) * 256))
    mosaic.save(path)
    json.dump({"z": z, "x0": x0, "y0": y0, "tiles_x": x1 - x0 + 1, "tiles_y": y1 - y0 + 1}, open(path.replace(".png", ".json"), "w"))
    print("elevation tiles:", (x1 - x0 + 1) * (y1 - y0 + 1))


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    fetch_religious()
    fetch_castles()
    fetch_rivers()
    fetch_elevation()
