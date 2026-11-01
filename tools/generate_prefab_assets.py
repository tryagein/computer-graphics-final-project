"""Generate the original, redistributable OBJ prefabs used by the course demo."""
from math import cos, pi, sin
from pathlib import Path
import random
import struct
import zlib

ROOT = Path(__file__).resolve().parents[1] / "assets" / "prefabs"
ROOT.mkdir(parents=True, exist_ok=True)


class Obj:
    def __init__(self, library):
        self.lines = [f"mtllib {library}"]
        self.count = 0

    def triangle(self, a, b, c):
        for p in (a, b, c):
            self.lines.append("v %.5f %.5f %.5f" % p)
        self.lines.append(f"f {self.count + 1} {self.count + 2} {self.count + 3}")
        self.count += 3

    def material(self, name):
        self.lines.append(f"usemtl {name}")

    def cylinder(self, center, bottom, top, radius_bottom, radius_top, sides=10):
        x, z = center
        for i in range(sides):
            a, b = 2 * pi * i / sides, 2 * pi * (i + 1) / sides
            p = (x + radius_bottom * cos(a), bottom, z + radius_bottom * sin(a))
            q = (x + radius_bottom * cos(b), bottom, z + radius_bottom * sin(b))
            r = (x + radius_top * cos(a), top, z + radius_top * sin(a))
            s = (x + radius_top * cos(b), top, z + radius_top * sin(b))
            self.triangle(p, r, q)
            self.triangle(q, r, s)

    def cone(self, center, bottom, top, radius, sides=11):
        x, z = center
        tip = (x, top, z)
        for i in range(sides):
            a, b = 2 * pi * i / sides, 2 * pi * (i + 1) / sides
            p = (x + radius * cos(a), bottom, z + radius * sin(a))
            q = (x + radius * cos(b), bottom, z + radius * sin(b))
            self.triangle(p, tip, q)

    def box(self, x0, y0, z0, x1, y1, z1):
        p = [(x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0),
             (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)]
        for a, b, c, d in ((0, 3, 2, 1), (4, 5, 6, 7), (0, 4, 7, 3),
                           (1, 2, 6, 5), (3, 7, 6, 2), (0, 1, 5, 4)):
            self.triangle(p[a], p[b], p[c])
            self.triangle(p[a], p[c], p[d])

    def write(self, name):
        (ROOT / name).write_text("\n".join(self.lines) + "\n", encoding="ascii")


pine = Obj("snow-pine.mtl")
pine.material("bark")
pine.cylinder((0, 0), 0, 1.7, .18, .11)
pine.material("needles")
for bottom, top, radius in ((.7, 2.05, .94), (1.18, 2.43, .78),
                            (1.68, 2.82, .59), (2.14, 3.14, .39)):
    pine.cone((0, 0), bottom, top, radius)
pine.material("snow")
for bottom, top, radius in ((1.37, 2.08, .52), (1.85, 2.46, .42),
                            (2.3, 2.86, .32), (2.72, 3.18, .22)):
    pine.cone((0, 0), bottom, top, radius)
for bottom, top, radius in ((.7, 2.05, .94), (1.18, 2.43, .78),
                            (1.68, 2.82, .59), (2.14, 3.14, .39)):
    pine.cone((0, 0), bottom + .06, bottom + .34,
              radius * (1 - .06 / (top - bottom)) + .04)
pine.write("snow-pine.obj")
(ROOT / "snow-pine.mtl").write_text(
    "newmtl bark\nKa .10 .06 .03\nKd .28 .16 .08\nKs .05 .05 .05\n"
    "newmtl needles\nKa .04 .07 .06\nKd .08 .25 .19\nKs .08 .12 .10\n"
    "newmtl snow\nKa .15 .17 .19\nKd .75 .84 .91\nKs .18 .20 .22\n", encoding="ascii")

lamp = Obj("street-lamp.mtl")
lamp.material("dark-metal")
lamp.cylinder((0, 0), 0, 2.76, .13, .085, 12)
lamp.cylinder((0, 0), 0, .10, .23, .18, 12)
lamp.box(-.09, 2.63, -.09, .78, 2.75, .09)
lamp.material("brass")
lamp.cylinder((.72, 0), 2.46, 2.66, .24, .16, 12)
lamp.cylinder((.72, 0), 3.05, 3.15, .20, .09, 12)
lamp.material("warm-glass")
lamp.cylinder((.72, 0), 2.65, 3.05, .15, .15, 12)
lamp.write("street-lamp.obj")
(ROOT / "street-lamp.mtl").write_text(
    "newmtl dark-metal\nKa .05 .05 .06\nKd .16 .19 .23\nKs .38 .42 .46\nNs 90\n"
    "newmtl brass\nKa .10 .07 .03\nKd .53 .36 .15\nKs .48 .40 .25\nNs 90\n"
    "newmtl warm-glass\nKa .12 .09 .05\nKd 1 .74 .38\nKe 1 .72 .40\nKs .15 .12 .08\n", encoding="ascii")

cabin = Obj("snow-cabin.mtl")
cabin.material("timber")
cabin.box(-1.25, 0, -1.0, 1.25, 1.75, 1.0)
cabin.material("door")
cabin.box(-.32, 0, 1.005, .32, 1.31, 1.045)
cabin.material("window-glass")
for x in (-.82, .82):
    cabin.box(x - .22, .85, 1.05, x + .22, 1.32, 1.08)
cabin.material("trim")
for x in (-.82, .82):
    for offset in (-.25, .25):
        cabin.box(x + offset - .025, .81, 1.08, x + offset + .025, 1.36, 1.12)
    cabin.box(x - .25, 1.06, 1.08, x + .25, 1.10, 1.12)
cabin.material("roof")
for z0, z1 in ((-1.16, 1.16),):
    cabin.triangle((-1.48, 1.71, z0), (-1.48, 1.71, z1), (0, 2.65, z0))
    cabin.triangle((-1.48, 1.71, z1), (0, 2.65, z1), (0, 2.65, z0))
    cabin.triangle((0, 2.65, z0), (0, 2.65, z1), (1.48, 1.71, z0))
    cabin.triangle((0, 2.65, z1), (1.48, 1.71, z1), (1.48, 1.71, z0))
cabin.material("snow")
cabin.triangle((-1.45, 1.80, -1.15), (-1.45, 1.80, 1.15), (-.08, 2.74, -1.15))
cabin.triangle((-1.45, 1.80, 1.15), (-.08, 2.74, 1.15), (-.08, 2.74, -1.15))
cabin.triangle((.08, 2.74, -1.15), (.08, 2.74, 1.15), (1.45, 1.80, -1.15))
cabin.triangle((.08, 2.74, 1.15), (1.45, 1.80, 1.15), (1.45, 1.80, -1.15))
cabin.write("snow-cabin.obj")
(ROOT / "snow-cabin.mtl").write_text(
    "newmtl timber\nKa .08 .05 .03\nKd .37 .23 .13\nKs .08 .07 .05\n"
    "newmtl door\nKa .04 .025 .015\nKd .16 .08 .04\nKs .04 .03 .02\n"
    "newmtl window-glass\nKa .10 .07 .04\nKd .85 .53 .24\nKe .70 .40 .16\n"
    "newmtl trim\nKa .10 .07 .04\nKd .48 .31 .16\nKs .10 .08 .06\n"
    "newmtl roof\nKa .04 .05 .06\nKd .12 .16 .20\nKs .10 .12 .15\n"
    "newmtl snow\nKa .15 .17 .19\nKd .80 .87 .94\nKs .15 .18 .20\n", encoding="ascii")

# A tiny deterministic snow-grain texture; ordinary PNG bytes keep the asset portable.
random.seed(3701)
size = 128
pixels = bytearray()
for _ in range(size):
    pixels.append(0)
    for _ in range(size):
        grain = random.randrange(-10, 11)
        pixels.extend((max(0, min(255, value + grain)) for value in (224, 234, 245)))


def chunk(kind, data):
    return (struct.pack(">I", len(data)) + kind + data +
            struct.pack(">I", zlib.crc32(kind + data) & 0xffffffff))


(ROOT / "snow-grain.png").write_bytes(
    b"\x89PNG\r\n\x1a\n" +
    chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 2, 0, 0, 0)) +
    chunk(b"IDAT", zlib.compress(bytes(pixels), 9)) + chunk(b"IEND", b""))
print("Generated original OBJ prefabs in", ROOT)
