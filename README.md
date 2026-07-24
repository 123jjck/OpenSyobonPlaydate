# OpenSyobonPlaydate

<img width="400" height="400" alt="Syobon Action running on a Playdate" src="https://github.com/user-attachments/assets/71e90701-fc13-4672-b398-b73b1e7bc6ee" />

A native C++ port of [OpenSyobonAction](https://github.com/akemin-dayo/OpenSyobonAction)
(Syobon Action / Cat Mario) for the [Playdate](https://play.date/).

All nine stages are included. Only stages 1–4 have been playtested; stages 5–9 may
be impossible to finish in this version.

Controls: d-pad to move, A or up to jump, hold B for 2× speed. On the title
screen, d-pad or crank selects a stage.

## Install

Download `SyobonAction.pdx.zip` from the
[latest release](https://github.com/123jjck/OpenSyobonPlaydate/releases/latest)
and sideload it via [play.date/account/sideload](https://play.date/account/sideload),
or extract it and copy `SyobonAction.pdx` to the `Games` folder in Data Disk mode.

## Build

Needs the Playdate SDK 3.1+, GNU Make, and the GNU Arm Embedded toolchain.

```sh
export PLAYDATE_SDK_PATH=/path/to/PlaydateSDK
make
```

This produces `SyobonAction.pdx` for both the Simulator and the device.

## Credits

Original Syobon Action by Chiku (ちく); SDL port by Mathew Velasquez;
OpenSyobonAction by Karen/明美 ([@akemin-dayo](https://github.com/akemin-dayo)).
Based on OpenSyobonAction RC3, commit
[`77ca2c1`](https://github.com/akemin-dayo/OpenSyobonAction/commit/77ca2c128784512eca10e24a453ca19c270a8af4).
