#!/usr/bin/env python3
"""
Crop transparency from PNG images.
Finds the bounding box of non-transparent pixels and crops to that region.
"""
from pathlib import Path
import sys

try:
    from PIL import Image
except ImportError:
    print("Pillow is required. Install with: pip install Pillow")
    sys.exit(1)


def get_content_bbox(img: Image.Image, black_as_empty: bool = True) -> tuple[int, int, int, int] | None:
    """
    Return (left, top, right, bottom) of content pixels.
    - First tries alpha channel (alpha=0 = transparent) for PNGs with real transparency
    - Falls back to black-as-empty for exports that use opaque black as padding
    """
    if img.mode != "RGBA":
        img = img.convert("RGBA")
    # Try alpha-based bbox first (handles true transparency)
    alpha = img.split()[3]
    bbox = alpha.getbbox()
    if bbox is not None:
        # Check if bbox is smaller than full image (i.e. there was padding)
        if bbox != (0, 0, img.width, img.height):
            return bbox
    # Fall back to black-as-empty for opaque-black padding
    pixels = img.load()
    w, h = img.size
    min_x, min_y = w, h
    max_x, max_y = 0, 0
    brightness_threshold = 25
    for y in range(h):
        for x in range(w):
            r, g, b, a = pixels[x, y]
            is_content = (max(r, g, b) > brightness_threshold) or (a < 250)
            if is_content:
                min_x = min(min_x, x)
                min_y = min(min_y, y)
                max_x = max(max_x, x)
                max_y = max(max_y, y)
    if min_x > max_x or min_y > max_y:
        return None
    return (min_x, min_y, max_x + 1, max_y + 1)


def crop_transparency(input_path: Path, output_path: Path | None = None) -> bool:
    """
    Crop transparent padding from a PNG. Saves in place if output_path is None.
    Returns True if cropping was performed, False if no change.
    """
    output_path = output_path or input_path
    img = Image.open(input_path).convert("RGBA")
    bbox = get_content_bbox(img)
    if bbox is None:
        return False
    left, top, right, bottom = bbox
    # If already tight, skip
    if left == 0 and top == 0 and right == img.width and bottom == img.height:
        return False
    cropped = img.crop(bbox)
    cropped.save(output_path, "PNG", optimize=True)
    return True


def main():
    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent
    # Check project assets/img first, then Cursor workspace assets
    assets_dirs = [
        project_root / "assets" / "img",
        project_root / "assets",
        Path(r"C:\Users\pursu\.cursor\projects\c-PROJECTS-A18-BloomVerb-BloomVerb\assets"),
    ]
    assets_dir = next((d for d in assets_dirs if d.exists()), None)
    if assets_dir is None:
        print(f"Assets directory not found: {assets_dir}")
        sys.exit(1)

    segmented = [
        "display_frame",
        "meter_frame",
        "fader_lane_bg",
        "strip_space_bg",
        "strip_bloom_bg",
        "strip_character_bg",
        "strip_tone_bg",
        "strip_utility_bg",
        "display2",
        "xtra1",
        "xtra2",
        "xtra3",
    ]

    pngs = list(assets_dir.glob("*.png")) + list(assets_dir.glob("**/*.png"))
    pngs = list(dict.fromkeys(pngs))  # dedupe (glob * matches same as ** in same dir)
    cropped_count = 0
    processed = 0
    # In assets/img, process all PNGs; elsewhere filter by segmented list
    process_all = "img" in str(assets_dir)

    for png in pngs:
        name = png.stem.lower()
        if not process_all and not any(seg in name for seg in segmented):
            continue
        processed += 1
        try:
            if crop_transparency(png):
                cropped_count += 1
                print(f"Cropped: {png.name}")
            else:
                print(f"Already tight: {png.name}")
        except Exception as e:
            print(f"Error {png.name}: {e}")

    if processed == 0:
        print("No matching segmented assets found.")
    elif cropped_count == 0:
        print(f"\nProcessed {processed} file(s), none needed cropping.")
    else:
        print(f"\nCropped {cropped_count} file(s).")


if __name__ == "__main__":
    main()
