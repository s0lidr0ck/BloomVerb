#!/usr/bin/env python3
"""
Convert greenscreen (#00FF00 or similar) to transparent in PNG images.
Use after generating knob assets with AI that doesn't output transparency correctly.
"""
from pathlib import Path
import sys

try:
    from PIL import Image
except ImportError:
    print("Pillow is required. Install with: pip install Pillow")
    sys.exit(1)


def green_to_transparent(
    input_path: Path,
    output_path: Path | None = None,
    green_threshold: int = 200,
    saturation_threshold: int = 100,
) -> bool:
    """
    Replace green pixels with transparency.
    A pixel is considered "green" if G is high and R/B are low (green-dominant).
    """
    output_path = output_path or input_path
    img = Image.open(input_path).convert("RGBA")
    pixels = img.load()
    w, h = img.size
    changed = 0

    for y in range(h):
        for x in range(w):
            r, g, b, a = pixels[x, y]
            # Green screen: G dominant, R and B low
            if g > green_threshold and r < green_threshold and b < green_threshold:
                pixels[x, y] = (0, 0, 0, 0)
                changed += 1
            # Also catch near-green (slightly off) - high G, R and B much lower
            elif g > 180 and g > r + 50 and g > b + 50:
                # Blend toward transparent based on how green it is
                greenness = min(1.0, (g - max(r, b)) / 150)
                new_alpha = int(255 * (1 - greenness))
                pixels[x, y] = (r, g, b, new_alpha)
                changed += 1

    img.save(output_path, "PNG", optimize=True)
    return changed > 0


def main():
    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent
    assets_dir = project_root / "assets" / "img"

    if not assets_dir.exists():
        print(f"Assets directory not found: {assets_dir}")
        sys.exit(1)

    # Process knob PNGs
    knob_files = list(assets_dir.glob("knob_*.png"))
    if not knob_files:
        print("No knob_*.png files found in assets/img")
        sys.exit(0)

    for png in knob_files:
        if green_to_transparent(png):
            print(f"Keyed: {png.name}")
        else:
            print(f"No green found: {png.name}")

    print(f"\nProcessed {len(knob_files)} file(s).")


if __name__ == "__main__":
    main()
