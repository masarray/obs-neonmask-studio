#!/usr/bin/env python3
"""Generate reproducible, owned RGBA PNGs for OBS NeonMask visual testing.

No third-party dependencies or redistributed reference photography. These
fixtures are test inputs, never simulated evidence of plugin rendering.
"""

import argparse
import hashlib
import json
import math
import struct
import zlib
from pathlib import Path

SIGNATURE = b"\x89PNG\r\n\x1a\n"
PALETTE = ((255, 36, 93), (255, 186, 40), (50, 224, 255),
           (90, 228, 115), (165, 83, 255), (255, 255, 255))
SPECS = (
    ("opaque-bars.png", 640, 360, "bars"),
    ("alpha-ramp.png", 640, 360, "alpha"),
    ("soft-edge.png", 640, 360, "edge"),
    ("checkerboard-small.png", 320, 180, "checker"),
    ("portrait-bars.png", 360, 640, "bars"),
    ("transparent.png", 320, 180, "clear"),
)


def pixel(kind, x, y, width, height):
    if kind == "clear":
        return (0, 0, 0, 0)
    if kind == "bars":
        return (*PALETTE[min(5, x * 6 // width)], 255)
    if kind == "checker":
        v = 225 if (x // 20 + y // 20) % 2 else 35
        return (v, v, v, 255)
    if kind == "alpha":
        a = x * 255 // (width - 1)
        # At zero alpha use zero RGB to make unwanted fringe easy to spot.
        return (0, 0, 0, 0) if not a else (255, y * 255 // (height - 1), 42, a)
    if kind == "edge":
        dx = (x - (width - 1) / 2) / (width * 0.32)
        dy = (y - (height - 1) / 2) / (height * 0.40)
        angle = math.atan2(dy, dx)
        radius = math.hypot(dx, dy)
        # Subtle fine contour detail approximates semi-transparent hair edges.
        coverage = max(0.0, min(1.0, (1.0 + 0.012 * math.sin(73 * angle) - radius) / 0.075))
        a = round(coverage * 255)
        return (0, 0, 0, 0) if not a else (134, 86, 64, a)
    raise ValueError("unknown fixture kind: " + kind)


def chunk(tag, body):
    return (struct.pack(">I", len(body)) + tag + body +
            struct.pack(">I", zlib.crc32(tag + body) & 0xFFFFFFFF))


def render_png(width, height, kind):
    rows = bytearray()
    for y in range(height):
        rows.append(0)  # PNG filter 0; no metadata or color-profile ambiguity.
        for x in range(width):
            rows.extend(pixel(kind, x, y, width, height))
    header = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    return (SIGNATURE + chunk(b"IHDR", header) +
            chunk(b"IDAT", zlib.compress(rows, level=9)) + chunk(b"IEND", b""))


def inspect_png(data, width, height):
    if not data.startswith(SIGNATURE):
        raise ValueError("PNG signature mismatch")
    offset, chunks = len(SIGNATURE), []
    while offset < len(data):
        if offset + 12 > len(data):
            raise ValueError("truncated PNG chunk")
        size = struct.unpack_from(">I", data, offset)[0]
        end = offset + 12 + size
        if end > len(data):
            raise ValueError("truncated PNG data")
        tag = data[offset + 4:offset + 8]
        body = data[offset + 8:offset + 8 + size]
        crc = struct.unpack_from(">I", data, offset + 8 + size)[0]
        if zlib.crc32(tag + body) & 0xFFFFFFFF != crc:
            raise ValueError("PNG CRC mismatch")
        chunks.append((tag, body))
        offset = end
    if [tag for tag, _ in chunks] != [b"IHDR", b"IDAT", b"IEND"]:
        raise ValueError("unexpected PNG chunks")
    if chunks[0][1] != struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0):
        raise ValueError("PNG dimensions or RGBA format mismatch")
    raw = zlib.decompress(chunks[1][1])
    stride = width * 4 + 1
    if len(raw) != stride * height:
        raise ValueError("PNG uncompressed length mismatch")
    stats = {"transparent": 0, "partial": 0, "opaque": 0}
    for y in range(height):
        row = raw[y * stride:(y + 1) * stride]
        if row[0] != 0:
            raise ValueError("unexpected PNG row filter")
        for x in range(width):
            r, g, b, a = row[1 + x * 4:5 + x * 4]
            if not a:
                stats["transparent"] += 1
                if r or g or b:
                    raise ValueError("transparent pixel has nonzero RGB")
            elif a == 255:
                stats["opaque"] += 1
            else:
                stats["partial"] += 1
    return stats


def generate(out):
    out.mkdir(parents=True, exist_ok=True)
    files = []
    for name, width, height, kind in SPECS:
        data = render_png(width, height, kind)
        stats = inspect_png(data, width, height)
        (out / name).write_bytes(data)
        files.append({"file": name, "width": width, "height": height,
                      "sha256": hashlib.sha256(data).hexdigest(), "alpha_pixels": stats})
    (out / "manifest.json").write_text(
        json.dumps({"format": "NeonMask owned visual fixtures v1", "files": files},
                   indent=2) + "\n", encoding="utf-8")


def verify(out):
    manifest = json.loads((out / "manifest.json").read_text(encoding="utf-8"))
    if manifest.get("format") != "NeonMask owned visual fixtures v1":
        raise ValueError("unexpected fixture manifest")
    if [item["file"] for item in manifest["files"]] != [spec[0] for spec in SPECS]:
        raise ValueError("fixture list mismatch")
    for item, spec in zip(manifest["files"], SPECS):
        name, width, height, kind = spec
        data = (out / name).read_bytes()
        stats = inspect_png(data, width, height)
        if (item["sha256"] != hashlib.sha256(data).hexdigest() or
                item["alpha_pixels"] != stats or
                item["width"] != width or item["height"] != height):
            raise ValueError("fixture manifest mismatch: " + name)
        if kind in ("alpha", "edge") and not stats["partial"]:
            raise ValueError("missing partial transparency: " + name)
        if kind == "clear" and stats["transparent"] != width * height:
            raise ValueError("clear fixture contains nontransparent pixels")
        if kind in ("bars", "checker") and stats["opaque"] != width * height:
            raise ValueError("opaque fixture contains transparency")
    print("PASS: six generated PNG fixtures, dimensions, CRC, RGBA, alpha and SHA256")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=Path, required=True, help="fixture output directory")
    parser.add_argument("--verify", action="store_true", help="re-read and validate all outputs")
    args = parser.parse_args()
    generate(args.out)
    if args.verify:
        verify(args.out)
    else:
        print("Generated six owned RGBA PNG fixtures at " + str(args.out))


if __name__ == "__main__":
    main()
