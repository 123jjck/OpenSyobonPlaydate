#!/usr/bin/env python3
"""Build Playdate launcher art from the converted 1-bit game assets."""

from __future__ import annotations

from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "Source"
DESTINATION = SOURCE / "SystemAssets"
LAUNCH_IMAGE_SOURCE = ROOT / "tools" / "assets" / "title-screen.png"


def load_sprite(
    filename: str,
    crop: tuple[int, int, int, int] | None = None,
) -> tuple[Image.Image, Image.Image]:
    """Load a converted asset without destroying its authored 1-bit pattern."""
    source = Image.open(SOURCE / "res" / filename).convert("RGBA")
    if crop is not None:
        source = source.crop(crop)

    monochrome = source.convert("L").point(
        lambda value: 255 if value >= 128 else 0,
        mode="1",
    )
    return monochrome, source.getchannel("A")


def resize_sprite(
    sprite: tuple[Image.Image, Image.Image],
    size: tuple[int, int],
) -> tuple[Image.Image, Image.Image]:
    image, mask = sprite
    return (
        image.resize(size, Image.Resampling.NEAREST),
        mask.resize(size, Image.Resampling.NEAREST),
    )


def paste_sprite(
    scene: Image.Image,
    sprite: tuple[Image.Image, Image.Image],
    position: tuple[int, int],
) -> None:
    image, mask = sprite
    scene.paste(image, position, mask)


def draw_ground(scene: Image.Image, ground_y: int) -> None:
    ground = load_sprite("brock.PNG", (33 * 5, 0, 33 * 5 + 30, 30))
    for x in range(0, scene.width, 29):
        paste_sprite(scene, ground, (x, ground_y))


def make_card() -> Image.Image:
    scene = Image.new("1", (350, 155), 1)

    logo = resize_sprite(load_sprite("syobon3.PNG"), (330, 49))
    paste_sprite(scene, logo, (10, 7))

    ground_y = 125
    player = load_sprite("player.PNG", (31 * 4, 0, 31 * 4 + 30, 36))
    spikes = load_sprite("haikei.PNG", (151, 0, 216, 29))
    hill = resize_sprite(
        load_sprite("haikei.PNG", (0, 0, 150, 90)),
        (105, 63),
    )
    question_block = load_sprite("brock.PNG", (33 * 2, 0, 33 * 2 + 30, 30))

    paste_sprite(scene, player, (42, ground_y - 36))
    paste_sprite(scene, spikes, (126, ground_y - 29))
    paste_sprite(scene, question_block, (208, ground_y - 42))
    paste_sprite(scene, hill, (255, ground_y - 63))
    draw_ground(scene, ground_y)
    return scene


def make_launch_image() -> Image.Image:
    source = Image.open(LAUNCH_IMAGE_SOURCE).convert("L")
    if source.size != (400, 240):
        raise ValueError(
            f"launch image must be 400x240, got {source.width}x{source.height}"
        )

    # Playdate screenshots use two display colors stored as RGB. Convert
    # those pixels back to a strict 1-bit image without introducing any
    # additional dithering or antialiasing.
    return source.point(lambda value: 255 if value >= 128 else 0, mode="1")


def main() -> None:
    DESTINATION.mkdir(parents=True, exist_ok=True)

    make_card().save(DESTINATION / "card.png")
    make_launch_image().save(DESTINATION / "launchImage.png")

    player, player_mask = load_sprite(
        "player.PNG",
        (31 * 4, 0, 31 * 4 + 30, 36),
    )
    icon = Image.new("1", (32, 32), 1)
    icon.paste(player, (1, -2), player_mask)
    icon.save(DESTINATION / "icon.png")


if __name__ == "__main__":
    main()
