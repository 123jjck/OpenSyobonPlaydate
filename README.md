# OpenSyobonPlaydate

![Syobon Action running on Playdate](Source/SystemAssets/launchImage.png)

A native C++ port of
[OpenSyobonAction](https://github.com/akemin-dayo/OpenSyobonAction) for
[Playdate](https://play.date/).

The port preserves the original game logic, traps, enemies, and all nine
stages while adapting the 480×420 game world to Playdate's 400×240,
1-bit display. It has been tested in Playdate Simulator and on real hardware.

> [!WARNING]
> Only stages 1–4 have been playtested. Stages 5–9 are included but may be
> impossible to complete in the current version.

## Features

- All nine OpenSyobonAction stages, selectable from the title screen
- Horizontal and vertical camera tracking for the smaller Playdate display
- Hand-tuned 1-bit graphics with dithering and hatching for readable scenery,
  platforms, pipes, and hazards
- English in-game messages with the original Japanese title artwork preserved
- Original music and sound effects converted to Playdate-friendly IMA ADPCM
- Contextual Playdate System Menu actions for restarting, returning to the
  title screen, and giving up during gameplay
- Rendering culling and static-screen redraw avoidance for real hardware
- The original whole-game double-speed mode, mapped to the B button

## Controls

| Control | Action |
| --- | --- |
| D-pad Left / Right | Move |
| A or D-pad Up | Jump; close an open message |
| Hold B | Run the entire game at 2× speed |
| D-pad Left / Right or crank, on the title screen | Select a stage |
| A, on the title screen | Start |
| Menu → Restart level | Restart from the beginning and clear the checkpoint |
| Menu → Title screen | Leave the current game |
| Menu → Give up | Die and respawn at the last checkpoint |

## Building

### Requirements

- Playdate SDK 3.1 or newer
- GNU Arm Embedded toolchain (`arm-none-eabi-g++`)
- GNU Make
- Clang on macOS, or GCC on Linux, for the Simulator binary

Clone the repository and point `PLAYDATE_SDK_PATH` at your SDK installation:

```sh
git clone https://github.com/123jjck/OpenSyobonPlaydate.git
cd OpenSyobonPlaydate
export PLAYDATE_SDK_PATH=/path/to/PlaydateSDK
make
```

The resulting `SyobonAction.pdx` contains binaries for both Playdate Simulator
and the device.

Useful build targets:

| Command | Result |
| --- | --- |
| `make` | Build and package Simulator and device binaries |
| `make run` | Build and open the game in Playdate Simulator |
| `make simulator` | Package only the Simulator binary |
| `make device` | Package only the device binary |
| `make clean` | Remove generated build products |

If `PLAYDATE_SDK_PATH` is not set, the Makefile also checks the SDK location in
`~/.Playdate/config`.

### Regenerating assets

Converted game assets are checked into the repository, so this step is not
required for a normal build.

To regenerate them, place an OpenSyobonAction checkout next to this repository
as `../OpenSyobonAction`, install Pillow and ffmpeg, then run:

```sh
make assets
```

## Installing on Playdate

Download `SyobonAction.pdx.zip` from the
[latest release](https://github.com/123jjck/OpenSyobonPlaydate/releases/latest),
then use either method:

- **Account → Sideload:** open the
  [Playdate Sideload page](https://play.date/account/sideload), sign in, and
  upload `SyobonAction.pdx.zip`. Refresh Game Library on the Playdate to
  download the game.
- **Data Disk:** extract `SyobonAction.pdx.zip`, put the Playdate into Data Disk
  mode, and copy `SyobonAction.pdx` into the `Games` folder on the mounted
  Playdate volume. Eject the Playdate safely, then launch the game.

See Playdate's
[official sideloading guide](https://help.play.date/games/sideloading/) for
additional details.

## Project layout

- `src/` — game code and the Playdate compatibility layer
- `Source/` — Playdate package metadata, graphics, music, and sound effects
- `tools/` — asset, audio, and launcher-art conversion scripts
- `CREDITS.md` — detailed source provenance

## Credits and provenance

- Original Syobon Action: Chiku (ちく)
- Open-source SDL port: Mathew Velasquez
- OpenSyobonAction fork and maintenance: Karen/明美
  ([@akemin-dayo](https://github.com/akemin-dayo))

This port is based on OpenSyobonAction RC3, commit
[`77ca2c1`](https://github.com/akemin-dayo/OpenSyobonAction/commit/77ca2c128784512eca10e24a453ca19c270a8af4).
The upstream snapshot asks distributors to credit the original developer but
does not include a standalone license file. This project therefore does not
assert a new license over imported game code, data, audio, or artwork. Please
review the upstream provenance and preserve these credits when redistributing
the project or its binaries.

This is an unofficial fan port and is not affiliated with Panic.
