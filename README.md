# lightshow-tree

Firmware and hardware notes for adapting a Member's Mark Pre-Lit Multicolor
Lightshow Tree to a Falcon differential output.

The first target retains the tree's factory power board, connector harness, and
30 vertical WS28xx data paths. One Falcon differential pixel stream carries the
1,020 vertical pixels; a small on-board RS-485 receiver converts it to the STC
MCU's 3.3 V input level.

## Status

Hardware reverse engineering is in progress. The initial `blank` firmware drives
the 31 confirmed pixel-data pins low (the WS28xx idle level), without emitting a
timed pixel frame or changing the factory power hardware. It exists to establish
a repeatable SDCC build and STC UART-ISP workflow before output timing is added.

## Layout

- `docs/` — confirmed board mapping, open questions, and ISP procedure
- `include/` — board-level declarations
- `src/` — firmware source
- `scripts/` — local development helpers

## Toolchain

The project targets the STC8H8K64U (8051-compatible) MCU.

- [SDCC](https://sdcc.sourceforge.net/) compiles the firmware.
- [stc8prog](https://github.com/IOsetting/stc8prog) detects and flashes the MCU
  over UART on macOS.

Once both are installed:

```sh
make
make blank
make mapper
make receiver
make check-toolchain
make detect PORT=/dev/cu.usbserial-XXXX
make terminal PORT=/dev/cu.usbserial-XXXX
```

`make detect` is non-destructive. See `docs/isp.md` before running it.
Exit the serial terminal with `Ctrl-A`, then `\\`.

The local `tools/stc8prog/` checkout is ignored by Git. It is the default
flashing tool while developing on this Mac; a globally installed copy can be
selected with `make STC8PROG=stc8prog ...`.

See [the hardware plan](docs/hardware-plan.md) for the selected MAX3485 and
base-mounted RJ45 breakout modules, wiring, and Falcon pair selection.

## Falcon receiver bring-up

`make receiver` builds the first RS-485 receiver firmware. It samples the
MAX3485 `RO` signal on STC pin 27 / P3.6, captures one 1,020-pixel RGB frame,
limits every component to 1/8 brightness, and replays the frame to the 30
verticals in their confirmed clockwise order.

Flash it only with the MAX3485 configured receive-only (`T` open and `EN` low
or open). The first valid capture emits `frame: captured and replayed` on the
UART. See `docs/receiver.md` for the first-hardware-test procedure and the
single sampling-delay value that may need adjustment.
