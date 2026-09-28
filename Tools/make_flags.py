"""
Draw the coats of arms the balls wear (Art/Heraldry/flag_<id>.png, 1024 x 1024).
    python Tools/make_flags.py
The ball material projects the square straight through the ball from the front (the eyes sit on the upper
middle), like a countryball flag. Flat heraldic colours with a slight painted variation; hand-painted art
can replace any of them later. The list of houses shown in game is in Source/.../Characters/Heraldry.cpp.
"""
import math
import os
import random
from PIL import Image, ImageDraw, ImageFilter

OUT = "Art/Heraldry"
S = 1024

# Heraldic tinctures, a little muted (painted, not neon)
ARGENT = (236, 232, 222)
GULES = (178, 34, 34)
OR = (222, 176, 52)
AZURE = (38, 74, 150)
SABLE = (34, 32, 32)
VERT = (46, 110, 52)
MURREY = (112, 32, 58)   # York's mulberry livery colour


def canvas(colour):
    img = Image.new("RGB", (S, S), colour)
    return img, ImageDraw.Draw(img)


def star(d, cx, cy, r, colour, points=5, inner=0.42):
    pts = []
    for i in range(points * 2):
        a = -math.pi / 2 + i * math.pi / points
        rr = r if i % 2 == 0 else r * inner
        pts.append((cx + rr * math.cos(a), cy + rr * math.sin(a)))
    d.polygon(pts, fill=colour)


def rose(d, cx, cy, r, petal, seed=OR):
    """Heraldic rose: five outer petals, five inner petals, a gold seed and green barbs between the petals."""
    for i in range(5):
        a = -math.pi / 2 + i * 2 * math.pi / 5 + math.pi / 5
        bx, by = cx + r * 0.95 * math.cos(a), cy + r * 0.95 * math.sin(a)
        star(d, bx, by, r * 0.28, VERT, points=3, inner=0.3)
    for i in range(5):
        a = -math.pi / 2 + i * 2 * math.pi / 5
        px, py = cx + r * 0.55 * math.cos(a), cy + r * 0.55 * math.sin(a)
        d.ellipse([px - r * 0.48, py - r * 0.48, px + r * 0.48, py + r * 0.48], fill=petal, outline=(60, 40, 30), width=6)
    for i in range(5):
        a = -math.pi / 2 + i * 2 * math.pi / 5 + math.pi / 5
        px, py = cx + r * 0.3 * math.cos(a), cy + r * 0.3 * math.sin(a)
        d.ellipse([px - r * 0.26, py - r * 0.26, px + r * 0.26, py + r * 0.26], fill=petal, outline=(60, 40, 30), width=4)
    d.ellipse([cx - r * 0.2, cy - r * 0.2, cx + r * 0.2, cy + r * 0.2], fill=seed, outline=(60, 40, 30), width=4)


def england():
    """St George: argent, a cross gules. English soldiers wore it as their badge."""
    img, d = canvas(ARGENT)
    w = int(S * 0.2)
    d.rectangle([S // 2 - w // 2, 0, S // 2 + w // 2, S], fill=GULES)
    d.rectangle([0, S // 2 - w // 2, S, S // 2 + w // 2], fill=GULES)
    return img


def york():
    """Livery of Richard, Duke of York: murrey and blue, the white rose of York."""
    img, d = canvas(MURREY)
    d.rectangle([S // 2, 0, S, S], fill=AZURE)
    rose(d, S / 2, S * 0.72, S * 0.2, ARGENT)
    return img


def lancaster():
    """Lancastrian livery: white and blue, the red rose of Lancaster."""
    img, d = canvas(ARGENT)
    d.rectangle([S // 2, 0, S, S], fill=AZURE)
    rose(d, S / 2, S * 0.72, S * 0.2, GULES)
    return img


def neville():
    """Neville (Earl of Salisbury, Earl of Warwick): gules, a saltire argent."""
    img, d = canvas(GULES)
    w = int(S * 0.2)
    d.line([(0, 0), (S, S)], fill=ARGENT, width=w)
    d.line([(S, 0), (0, S)], fill=ARGENT, width=w)
    return img


def stafford():
    """Stafford (Duke of Buckingham): or, a chevron gules."""
    img, d = canvas(OR)
    w = S * 0.17
    d.polygon([(0, S * 0.95), (S / 2, S * 0.45), (S, S * 0.95), (S, S * 0.95 + w), (S / 2, S * 0.45 + w), (0, S * 0.95 + w)], fill=GULES)
    return img


def clifford():
    """Clifford (Lord Clifford): chequy or and azure, a fess gules."""
    img, d = canvas(OR)
    n = 8
    c = S / n
    for x in range(n):
        for y in range(n):
            if (x + y) % 2:
                d.rectangle([x * c, y * c, (x + 1) * c, (y + 1) * c], fill=AZURE)
    d.rectangle([0, S * 0.62, S, S * 0.8], fill=GULES)
    return img


def courtenay():
    """Courtenay (Earl of Devon): or, three torteaux, a label azure."""
    img, d = canvas(OR)
    r = int(S * 0.12)
    for cx, cy in ((0.3, 0.62), (0.7, 0.62), (0.5, 0.84)):
        d.ellipse([cx * S - r, cy * S - r, cx * S + r, cy * S + r], fill=GULES)
    top, h = int(S * 0.12), int(S * 0.06)
    d.rectangle([int(S * 0.1), top, int(S * 0.9), top + h], fill=AZURE)
    for cx in (0.25, 0.5, 0.75):
        d.rectangle([int(cx * S - S * 0.04), top, int(cx * S + S * 0.04), top + int(S * 0.15)], fill=AZURE)
    return img


def bonville():
    """Bonville (Lord Bonville): sable, six mullets argent, 3, 2 and 1."""
    img, d = canvas(SABLE)
    for cx, cy in ((0.2, 0.47), (0.5, 0.47), (0.8, 0.47), (0.35, 0.67), (0.65, 0.67), (0.5, 0.87)):
        star(d, cx * S, cy * S, S * 0.085, ARGENT)
    return img


def scrope():
    """Scrope of Bolton: azure, a bend or."""
    img, d = canvas(AZURE)
    d.line([(-S * 0.1, -S * 0.1), (S * 1.1, S * 1.1)], fill=OR, width=int(S * 0.24))
    return img


def vere():
    """de Vere (Earl of Oxford): quarterly gules and or, in the first quarter a mullet argent."""
    img, d = canvas(GULES)
    d.rectangle([S // 2, 0, S, S // 2], fill=OR)
    d.rectangle([0, S // 2, S // 2, S], fill=OR)
    star(d, S * 0.22, S * 0.22, S * 0.1, ARGENT)
    return img


FLAGS = [("england", england), ("york", york), ("lancaster", lancaster), ("neville", neville), ("stafford", stafford),
         ("clifford", clifford), ("courtenay", courtenay), ("bonville", bonville), ("scrope", scrope), ("vere", vere)]


def painted(img, seed):
    """Subtle brush-like variation so flat colours don't look plastic."""
    rnd = random.Random(seed)
    small = Image.new("L", (S // 16, S // 16))
    small.putdata([rnd.randint(-14, 14) + 128 for _ in range(small.width * small.height)])
    noise = small.resize((S, S), Image.BICUBIC).filter(ImageFilter.GaussianBlur(10))
    out = img.copy()
    px, nz = out.load(), noise.load()
    for y in range(S):
        for x in range(S):
            k = (nz[x, y] - 128) / 255.0
            r, g, b = px[x, y]
            px[x, y] = (max(0, min(255, int(r * (1 + k)))), max(0, min(255, int(g * (1 + k)))), max(0, min(255, int(b * (1 + k)))))
    return out


def main():
    os.makedirs(OUT, exist_ok=True)
    for index, (name, make) in enumerate(FLAGS):
        painted(make(), seed=index).save(os.path.join(OUT, f"flag_{name}.png"))
        print("flag", name)


if __name__ == "__main__":
    main()
