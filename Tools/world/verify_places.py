"""
Check every place in Data/World/Places_England1455.csv against its Wikipedia coordinates and fix any that are
more than 1 km off.   python Tools/world/verify_places.py
Wikipedia titles come from the place name, with the overrides below for ambiguous ones.
"""
import csv
import json
import math
import time
import urllib.parse
import urllib.request

PATH = "Data/World/Places_England1455.csv"
UA = "CrownsAndCommoners-dev/0.1 (indie game prototype; map planning)"

TITLES = {
    "london": "City of London", "westminster": "Palace of Westminster", "york": "York", "bristol": "Bristol",
    "richmond": "Richmond, North Yorkshire", "newcastle": "Newcastle upon Tyne", "hull": "Kingston upon Hull",
    "kings_lynn": "King's Lynn", "sandwich": "Sandwich, Kent", "bath": "Bath, Somerset", "ludlow": "Ludlow",
    "ware": "Ware, Hertfordshire", "barnet": "Chipping Barnet", "stamford": "Stamford, Lincolnshire",
    "newark": "Newark-on-Trent", "huntingdon": "Huntingdon", "lancaster": "Lancaster, Lancashire",
    "kendal": "Kendal", "berwick": "Berwick-upon-Tweed", "warwick": "Warwick", "stafford": "Stafford",
    "middleham": "Middleham", "skipton": "Skipton", "wakefield": "Wakefield", "pontefract": "Pontefract",
    "doncaster": "Doncaster", "tadcaster": "Tadcaster", "ripon": "Ripon", "tewkesbury": "Tewkesbury",
    "boston": "Boston, Lincolnshire", "derby": "Derby", "windsor": "Windsor Castle", "sandal": "Sandal Castle",
    "pontefract_castle": "Pontefract Castle", "sheriff_hutton": "Sheriff Hutton Castle",
    "fotheringhay": "Fotheringhay Castle", "bury": "Bury St Edmunds Abbey", "walsingham": "Walsingham",
    "st_albans_abbey": "St Albans Cathedral", "fountains": "Fountains Abbey", "jervaulx": "Jervaulx Abbey",
    "bolton_priory": "Bolton Priory", "glastonbury": "Glastonbury Abbey", "rievaulx": "Rievaulx Abbey",
    "westminster_abbey": "Westminster Abbey", "tower_london": "Tower of London",
    "battle_st_albans_1455": "First Battle of St Albans", "battle_st_albans_1461": "Second Battle of St Albans",
    "battle_blore_heath": "Battle of Blore Heath", "battle_ludford": "Battle of Ludford Bridge",
    "battle_northampton": "Battle of Northampton (1460)", "battle_wakefield": "Battle of Wakefield",
    "battle_mortimers_cross": "Battle of Mortimer's Cross", "battle_ferrybridge": "Battle of Ferrybridge",
    "battle_towton": "Battle of Towton", "battle_hedgeley_moor": "Battle of Hedgeley Moor",
    "battle_hexham": "Battle of Hexham", "battle_edgecote": "Battle of Edgcote",
    "battle_losecote": "Battle of Losecote Field", "battle_barnet": "Battle of Barnet",
    "battle_tewkesbury": "Battle of Tewkesbury", "battle_bosworth": "Battle of Bosworth Field",
    "battle_stoke": "Battle of Stoke Field", "towton": "Towton", "coverham": "Coverham",
    "masham": "Masham", "leyburn": "Leyburn", "raby": "Raby Castle",
    "carlisle_castle": "Carlisle Castle", "nottingham_castle": "Nottingham Castle", "canterbury": "Canterbury",
    "plymouth": "Plymouth", "lincoln": "Lincoln, England", "worcester": "Worcester, England",
    "durham": "Durham, England", "saxton": "Saxton, North Yorkshire", "wensley": "Wensley, North Yorkshire", "exeter": "Exeter", "whitby": "Whitby", "scarborough": "Scarborough, North Yorkshire",
}


def title_for(row):
    if row["Id"] in TITLES:
        return TITLES[row["Id"]]
    return row["Name"]


def fetch(titles):
    q = urllib.parse.urlencode({"action": "query", "prop": "coordinates", "titles": "|".join(titles),
                                "redirects": 1, "format": "json", "colimit": "max"})
    req = urllib.request.Request("https://en.wikipedia.org/w/api.php?" + q, headers={"User-Agent": UA})
    for attempt in range(4):
        try:
            data = json.load(urllib.request.urlopen(req, timeout=30))
            break
        except Exception:
            time.sleep(3 * (attempt + 1))
    names = {t: t for t in titles}
    for r in data["query"].get("normalized", []) + data["query"].get("redirects", []):
        for k, v in list(names.items()):
            if v == r["from"]:
                names[k] = r["to"]
    coords = {}
    for p in data["query"]["pages"].values():
        if "coordinates" in p:
            coords[p["title"]] = (p["coordinates"][0]["lat"], p["coordinates"][0]["lon"])
    return {t: coords.get(names[t]) for t in titles}


def dist_km(a, b):
    return math.hypot((a[0] - b[0]) * 110.57, (a[1] - b[1]) * 111.32 * math.cos(math.radians(a[0])))


all_rows = list(csv.DictReader(open(PATH, encoding="utf-8")))
fields = list(all_rows[0].keys())
# Places from Wikidata (rh_, c_) already carry authoritative coordinates
# Region labels (Area) are placed by hand at the middle of the region, not at Wikipedia's pin
rows = [r for r in all_rows if not r["Id"].startswith(("rh_", "c_")) and r["Type"] != "Area"]
titles = [title_for(r) for r in rows]
found = {}
for i in range(0, len(titles), 45):
    found.update(fetch(titles[i:i + 45]))
    time.sleep(1)

fixed, missing = [], []
for row, t in zip(rows, titles):
    c = found.get(t)
    if not c:
        missing.append(row["Id"])
        continue
    old = (float(row["Lat"]), float(row["Lon"]))
    d = dist_km(old, c)
    if d > 1.0:
        fixed.append(f'{row["Id"]}: moved {d:.1f} km')
        row["Lat"], row["Lon"] = f"{c[0]:.4f}", f"{c[1]:.4f}"

w = csv.DictWriter(open(PATH, "w", encoding="utf-8", newline=""), fieldnames=fields)
w.writeheader()
w.writerows(all_rows)
print(f"{len(rows)} places, {len(fixed)} corrected, {len(missing)} not found on Wikipedia")
print("\n".join(fixed))
print("not found:", ", ".join(missing))
