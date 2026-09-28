"""
Download coats of arms from Wikimedia Commons (rendered to PNG by Wikimedia) and turn each into a square
"flag" for the ball: the shield is cropped to its top square and the empty corners are filled from the field,
so the arms cover the whole ball like a countryball flag.
    python Tools/fetch_wikimedia_arms.py
Writes Art/Heraldry/flag_<id>.png (overwrites the code-drawn placeholder), keeps the original download in
Art/Heraldry/Source/ and logs source, author and licence in Art/Heraldry/SOURCES.md.
"""
import io
import json
import os
import re
import time
import urllib.parse
import urllib.request

from PIL import Image, ImageFilter

OUT = "Art/Heraldry"
SOURCE = os.path.join(OUT, "Source")
UA = "CountriesIRL-dev/0.1 (indie game prototype; heraldry textures)"
S = 1024

# id -> Wikimedia file. York and Lancaster keep the livery versions drawn by Tools/make_flags.py
FILES = {
    "england": "File:Flag of England.svg",
    "neville": "File:Neville arms.svg",
    "stafford": "File:Stafford arms.svg",
    "clifford": "File:Arms of Clifford.svg",
    "courtenay": "File:Arms of the House of Courtenay, earls of Devon (label).svg",
    "bonville": "File:Arms of Bonville.svg",
    "scrope": "File:Scrope arms.svg",
    "vere": "File:Arms of de Vere.svg",
    "percy": "File:Modern arms of Percy.svg",
    "mowbray": "File:Arms of Mowbray.svg",
    "talbot": "File:Coat of Arms of John Talbot, 1st Earl of Shrewsbury.svg",
}


def get(url):
    req = urllib.request.Request(url, headers={"User-Agent": UA})
    for attempt in range(5):
        try:
            with urllib.request.urlopen(req, timeout=30) as r:
                return r.read()
        except Exception as e:  # rate limits: wait and retry
            time.sleep(4 * (attempt + 1))
            last = e
    raise last


def info(title):
    q = urllib.parse.urlencode({"action": "query", "prop": "imageinfo", "iiprop": "url|extmetadata",
                                "iiurlwidth": S, "format": "json", "titles": title})
    page = next(iter(json.loads(get("https://commons.wikimedia.org/w/api.php?" + q))["query"]["pages"].values()))
    i = page["imageinfo"][0]
    meta = i.get("extmetadata", {})
    clean = lambda k: re.sub(r"<[^>]+>", "", meta.get(k, {}).get("value", "")).strip()
    return i["thumburl"], i["descriptionurl"], clean("LicenseShortName"), clean("Artist") or "unknown"


def to_flag(img):
    """Shield -> square: trim the outer outline, take the top square, fill empty corners from each row's field."""
    img = img.convert("RGBA")
    box = img.getchannel("A").point(lambda a: 255 if a > 128 else 0).getbbox()
    img = img.crop(box)
    w, h = img.size
    if abs(w - h) / max(w, h) > 0.05:          # a shield: drop the outline and keep the top square
        m = int(w * 0.04)
        img = img.crop((m, m, w - m, min(h - m, m + (w - 2 * m))))
    img = img.resize((S, S), Image.LANCZOS)
    # Inside the shield = opaque, shrunk a little so the dark outline around the shield counts as outside
    inside = img.getchannel("A").point(lambda a: 255 if a > 200 else 0).filter(ImageFilter.MinFilter(41))
    px, ok = img.load(), inside.load()
    for y in range(S):
        row = [x for x in range(S) if ok[x, y]]
        if not row:
            for x in range(S):
                px[x, y] = px[x, y - 1] if y > 0 else (128, 128, 128, 255)
            continue
        for x in range(S):
            if not ok[x, y]:
                src = row[0] if x < row[0] else row[-1]
                px[x, y] = px[src, y]
    return img.convert("RGB")


def main():
    os.makedirs(SOURCE, exist_ok=True)
    lines = ["# Coats of arms: sources and licences", "",
             "CC BY-SA images: credit the artist; our square versions are shared under the same licence.",
             "**Replace CC BY-SA arms with our own art before a commercial release** (grey area for games).", "",
             "| Game file | Wikimedia file | Author | Licence |", "|---|---|---|---|"]
    for key, title in FILES.items():
        thumb, page, licence, artist = info(title)
        raw = get(thumb)
        with open(os.path.join(SOURCE, f"{key}.png"), "wb") as f:
            f.write(raw)
        to_flag(Image.open(io.BytesIO(raw))).save(os.path.join(OUT, f"flag_{key}.png"))
        lines.append(f"| flag_{key}.png | [{title[5:]}]({page}) | {artist} | {licence} |")
        print("arms", key, "|", licence, "|", artist)
        time.sleep(2)
    lines += ["| flag_york.png, flag_lancaster.png | drawn by Tools/make_flags.py (livery colours) | CountriesIRL | ours |", ""]
    with open(os.path.join(OUT, "SOURCES.md"), "w", encoding="utf-8") as f:
        f.write("\n".join(lines))


if __name__ == "__main__":
    main()
