#!/usr/bin/env python3
"""Convert the OpenSyobonAction sprite sheets for Playdate.

The SDL port uses RGB cyan (153, 255, 255) as its transparency key. Playdate
expects a real alpha channel, so this converts that key to transparent pixels.
Opaque colour pixels are converted to a small set of deterministic diagonal
hatching patterns instead of leaving pdc to flatten every colour to solid
black or white.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image


TRANSPARENT_KEY = (153, 255, 255)
TITLE_BACKGROUND = (160, 180, 250)
HATCH_PHASES = {
    0: (),
    1: (0,),
    2: (0, 4),
    3: (0, 3, 6),
    4: (0, 1, 4, 5),
    6: (0, 1, 3, 4, 5, 7),
    8: tuple(range(8)),
}


def ink_density(luminance: int) -> int:
    """Quantize luminance to one of seven hand-tuned hatch densities."""

    if luminance >= 232:
        return 0
    if luminance >= 192:
        return 1
    if luminance >= 152:
        return 2
    if luminance >= 112:
        return 3
    if luminance >= 80:
        return 4
    if luminance >= 48:
        return 6
    return 8


def hatched_monochrome(density: int, x: int, y: int) -> int:
    """Draw coherent diagonal strokes with density eighths of black ink."""

    phase = (x + y) % 8
    return 0 if phase in HATCH_PHASES[density] else 255


def title_monochrome(red: int, green: int, blue: int, x: int, y: int) -> int | None:
    """Remove the title's sky-coloured matte and hatch its orange lettering."""

    distance_squared = (
        (red - TITLE_BACKGROUND[0]) ** 2
        + (green - TITLE_BACKGROUND[1]) ** 2
        + (blue - TITLE_BACKGROUND[2]) ** 2
    )
    if distance_squared < 52**2:
        return None

    is_warm_letter_fill = red > green + 30 and red > blue + 70
    density = 4 if is_warm_letter_fill else 8
    return hatched_monochrome(density, x, y)


def convert_image(source: Path, destination: Path) -> None:
    image = Image.open(source).convert("RGBA")
    pixels = image.load()
    is_title = source.name == "syobon3.PNG"

    for y in range(image.height):
        for x in range(image.width):
            red, green, blue, _ = pixels[x, y]
            if (red, green, blue) == TRANSPARENT_KEY:
                pixels[x, y] = (255, 255, 255, 0)
                continue

            if is_title:
                value = title_monochrome(red, green, blue, x, y)
                if value is None:
                    pixels[x, y] = (255, 255, 255, 0)
                    continue
            else:
                luminance = (red * 299 + green * 587 + blue * 114) // 1000
                value = hatched_monochrome(ink_density(luminance), x, y)

            pixels[x, y] = (value, value, value, 255)

    destination.parent.mkdir(parents=True, exist_ok=True)
    image.save(destination)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    arguments = parser.parse_args()

    for source in sorted(arguments.source.glob("*.PNG")):
        convert_image(source, arguments.destination / source.name)


if __name__ == "__main__":
    main()
