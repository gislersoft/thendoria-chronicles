from __future__ import annotations

import argparse
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description="Convert PCX assets to PNG recursively.")
    parser.add_argument("--src", default="../IMG", help="Source folder with .pcx files")
    parser.add_argument("--dst", default="../assets_png/IMG", help="Destination folder for .png files")
    args = parser.parse_args()

    try:
        from PIL import Image
    except ImportError:
        print("Pillow is required. Install with: py -m pip install pillow")
        return 1

    src_root = Path(args.src).resolve()
    dst_root = Path(args.dst).resolve()

    if not src_root.exists():
        print(f"Source folder does not exist: {src_root}")
        return 1

    pcx_files = sorted(list(src_root.rglob("*.pcx")) + list(src_root.rglob("*.PCX")))
    if not pcx_files:
        print(f"No PCX files found under: {src_root}")
        return 0

    converted = 0
    for pcx_path in pcx_files:
        rel = pcx_path.relative_to(src_root)
        out_path = (dst_root / rel).with_suffix(".png")
        out_path.parent.mkdir(parents=True, exist_ok=True)

        with Image.open(pcx_path) as img:
            img = img.convert("RGBA")
            img.save(out_path, "PNG")

        converted += 1
        print(f"Converted: {pcx_path} -> {out_path}")

    print(f"Done. Converted {converted} file(s).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
