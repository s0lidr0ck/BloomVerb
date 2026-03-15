#!/usr/bin/env python3
"""
Center knob images for correct rotation in JUCE.

When a knob isn't perfectly round (e.g. has an indicator protrusion), the rotation
center must be the center of the circular hub, not the bounding box center.
This script creates a square canvas with your specified rotation center at the
middle, so JUCE's rotate-around-center works correctly.

Usage:
  python center_knob_for_rotation.py                    # auto-detect center (bbox)
  python center_knob_for_rotation.py 92 92              # use (92, 92) as rotation center
  python center_knob_for_rotation.py knob_bronze.png 96 88  # specific file + center
"""
from pathlib import Path
import sys

try:
    from PIL import Image
except ImportError:
    print("Pillow is required. Install with: pip install Pillow")
    sys.exit(1)


def get_content_bbox(img: Image.Image, treat_as_empty: str = "both") -> tuple[int, int, int, int] | None:
    """Get bbox of non-empty pixels. treat_as_empty: 'alpha' | 'black' | 'both'"""
    if img.mode != "RGBA":
        img = img.convert("RGBA")
    pixels = img.load()
    w, h = img.size
    min_x, min_y = w, h
    max_x, max_y = 0, 0

    for y in range(h):
        for x in range(w):
            r, g, b, a = pixels[x, y]
            if treat_as_empty == "alpha" and a < 10:
                continue
            if treat_as_empty == "black" and max(r, g, b) < 25:
                continue
            if treat_as_empty == "both" and (a < 10 or (max(r, g, b) < 25 and a > 250)):
                continue
            min_x, min_y = min(min_x, x), min(min_y, y)
            max_x, max_y = max(max_x, x), max(max_y, y)

    if min_x > max_x or min_y > max_y:
        return None
    return (min_x, min_y, max_x + 1, max_y + 1)


def center_knob(
    input_path: Path,
    output_path: Path | None = None,
    center_x: int | None = None,
    center_y: int | None = None,
    size: int = 184,
) -> bool:
    """
    Create a square image with the rotation center at the middle.
    The knob is placed so (center_x, center_y) in the source ends up at (size/2, size/2).
    """
    output_path = output_path or input_path
    img = Image.open(input_path).convert("RGBA")

    if center_x is None or center_y is None:
        # Use image center—for knobs with a protrusion, the hub is usually at geometric center
        center_x = img.width // 2
        center_y = img.height // 2
        print(f"  Using image center: ({center_x}, {center_y})")

    # Create square canvas (transparent)
    canvas = Image.new("RGBA", (size, size), (0, 0, 0, 0))

    # Paste the knob so its rotation center is at canvas center
    # Source (center_x, center_y) should end up at (size/2, size/2)
    paste_x = size // 2 - center_x
    paste_y = size // 2 - center_y
    canvas.paste(img, (paste_x, paste_y), img)

    canvas.save(output_path, "PNG", optimize=True)
    return True


def main():
    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent
    assets_dir = project_root / "assets" / "img"

    if not assets_dir.exists():
        print(f"Assets directory not found: {assets_dir}")
        sys.exit(1)

    # Parse args: [file] [cx] [cy]
    args = sys.argv[1:]
    center_x = None
    center_y = None
    target_files = None

    if len(args) >= 2 and args[-2].isdigit() and args[-1].isdigit():
        center_y = int(args.pop())
        center_x = int(args.pop())
    if args and args[0].endswith(".png"):
        target_files = [assets_dir / args[0]] if (assets_dir / args[0]).exists() else []
    if target_files is None:
        target_files = list(assets_dir.glob("knob_*.png"))

    if not target_files:
        print("No knob_*.png files found in assets/img")
        if center_x is not None:
            print("Usage: center_knob_for_rotation.py [knob_xxx.png] [center_x] [center_y]")
        sys.exit(0)

    print(f"Rotation center: {'auto' if center_x is None else f'({center_x}, {center_y})'}")
    print(f"Output size: 184 x 184\n")

    for png in target_files:
        if center_knob(png, center_x=center_x, center_y=center_y):
            print(f"Centered: {png.name}")

    print(f"\nDone. When JUCE rotates these, use the image center as the rotation origin.")


if __name__ == "__main__":
    main()
