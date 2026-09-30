"""
Build the game's terrain (a heightmap of the first release's England) from real elevation.
    python Tools/world/build_terrain.py
Reads Art/MapData/elevation_z8.png (AWS Terrain Tiles, fetch_map_data.py). Writes
  Data/World/Terrain_England1455.png   16-bit heightmap in GAME metres (Unreal can import 16-bit PNG heightmaps)
  Data/World/Terrain_England1455.json  its bounds, pixel size and height scale
The real land is generalised for the game: small bumps are smoothed away (they shrink to nothing at 1:12), and heights
are squashed by landform, so vales and fens stay flat, hills and downs stay gentle and moors and mountains stay
dramatic instead of turning into cliffs (heights squashed less than distances) or going flat (squashed as much).
"""
import os
import json
import math

import numpy as np
from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))).replace("\\", "/")   # the project folder
DATA = f"{ROOT}/Art/MapData"
COMPRESSION = float(json.load(open(f"{ROOT}/Data/World/World_Release1.json", encoding="utf-8"))["compression"])
BOX = (-3.8, 50.5, 1.8, 55.83)          # lon/lat of the first release's England (as the planning map)
LAT0, LON0 = 53.0, -1.9
KX = 111.32 * math.cos(math.radians(LAT0))
KY = 110.57
CELL_KM = 0.25                          # real km per heightmap pixel (~21 m of game world)
SMOOTH_KM = 1.6                         # real bumps smaller than this are smoothed away (~80 m in the game)

# Real height (m) -> game height (m): a curve, not one factor. Pairs of (real, game); straight lines in between.
#   vales and fens (0-60 m) nearly flat, hills and downs (60-300 m) gentle, moors and mountains (300 m+) steeper.
# (Set for 1:20 on 2026-09-29: three quarters of the 1:12 curve, so slopes stay walkable with the land squeezed more.)
HEIGHT_CURVE = [(0, 0), (60, 4.5), (150, 16.5), (300, 41), (500, 90), (700, 142), (1000, 210)]


def game_height(real):
    xs, ys = zip(*HEIGHT_CURVE)
    return np.interp(real, xs, ys)


def km(lon, lat):
    return (lon - LON0) * KX, (lat - LAT0) * KY


def gaussian_blur(a, sigma):
    r = int(3 * sigma)
    k = np.exp(-0.5 * (np.arange(-r, r + 1) / sigma) ** 2)
    k /= k.sum()
    a = np.apply_along_axis(lambda v: np.convolve(np.pad(v, r, mode="edge"), k, mode="valid"), 0, a)
    return np.apply_along_axis(lambda v: np.convolve(np.pad(v, r, mode="edge"), k, mode="valid"), 1, a)


def real_heights():
    """Real elevation (m) on a grid of CELL_KM over BOX, north at the top."""
    meta = json.load(open(f"{DATA}/elevation_z8.json"))
    tiles = np.asarray(Image.open(f"{DATA}/elevation_z8.png").convert("RGB"), dtype=np.float64)
    elev = tiles[..., 0] * 256 + tiles[..., 1] + tiles[..., 2] / 256 - 32768
    x0, y0 = km(BOX[0], BOX[1])
    x1, y1 = km(BOX[2], BOX[3])
    w, h = int((x1 - x0) / CELL_KM), int((y1 - y0) / CELL_KM)
    xs = x0 + (np.arange(w) + 0.5) * CELL_KM
    ys = y1 - (np.arange(h) + 0.5) * CELL_KM
    lon = xs / KX + LON0
    lat = ys / KY + LAT0
    n = 2 ** meta["z"]
    fx = ((lon + 180) / 360 * n - meta["x0"]) * 256
    latr = np.radians(lat)
    fy = ((1 - np.log(np.tan(latr) + 1 / np.cos(latr)) / math.pi) / 2 * n - meta["y0"]) * 256
    FX, FY = np.meshgrid(fx, fy)
    ix, iy = np.clip(FX.astype(int), 0, elev.shape[1] - 2), np.clip(FY.astype(int), 0, elev.shape[0] - 2)
    tx, ty = FX - ix, FY - iy                                  # bilinear
    e = (elev[iy, ix] * (1 - tx) * (1 - ty) + elev[iy, ix + 1] * tx * (1 - ty)
         + elev[iy + 1, ix] * (1 - tx) * ty + elev[iy + 1, ix + 1] * tx * ty)
    return np.maximum(e, 0), (x0, y0, x1, y1)


def main():
    real, (x0, y0, x1, y1) = real_heights()
    smooth = gaussian_blur(real, SMOOTH_KM / CELL_KM / 2)
    game = game_height(smooth)
    top = float(game.max())
    scale = 65535 / 400.0                                      # 16-bit value per game metre (0-400 m range)
    Image.fromarray(np.round(game * scale).astype(np.uint16)).save(f"{ROOT}/Data/World/Terrain_England1455.png")
    json.dump({"box_lonlat": BOX, "projection": {"lat0": LAT0, "lon0": LON0, "km_per_deg_lon": KX, "km_per_deg_lat": KY},
               "cell_real_km": CELL_KM, "cell_game_m": CELL_KM * 1000 / COMPRESSION,
               "size_px": [game.shape[1], game.shape[0]], "value_per_game_m": scale,
               "height_curve_real_to_game_m": HEIGHT_CURVE, "smooth_real_km": SMOOTH_KM,
               "highest_game_m": round(top, 1)},
              open(f"{ROOT}/Data/World/Terrain_England1455.json", "w"), indent=1)
    for name, lon, lat in [("Scafell Pike", -3.211, 54.454), ("Cross Fell (Pennines)", -2.487, 54.703),
                           ("Kinder Scout (Peak)", -1.872, 53.385), ("Cheviot", -2.145, 55.478),
                           ("North York Moors", -1.0, 54.39), ("Chilterns", -0.72, 51.72), ("The Fens", 0.05, 52.5),
                           ("Middleham", -1.806, 54.285), ("Vale of York", -1.25, 54.0)]:
        x, y = km(lon, lat)
        i, j = int((x - x0) / CELL_KM), int((y1 - y) / CELL_KM)
        print(f"{name:24s} real {real[j, i]:5.0f} m  smoothed {smooth[j, i]:5.0f} m  -> game {game[j, i]:5.0f} m")
    print(f"heightmap {game.shape[1]}x{game.shape[0]} px, {CELL_KM * 1000 / COMPRESSION:.0f} m of game per px, highest {top:.0f} m")


if __name__ == "__main__":
    main()
