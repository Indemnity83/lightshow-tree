# lightshow-tree

Firmware and hardware notes for adapting a Member's Mark Pre-Lit Multicolor
Lightshow Tree to a Falcon differential output.

The first target retains the tree's factory power board, connector harness, and
30 vertical WS28xx data paths. Two Falcon differential pixel outputs carry the
1,020 vertical pixels, 510 each; two small on-board RS-485 receivers convert
them to the STC MCU's 3.3 V input level.

## Status

The pixel-output mapping is confirmed, and the two-channel receiver drives all
30 verticals from two Falcon outputs: a Falcon chase test runs cleanly across
the seam between verticals 15 and 16. The star and a fourth output are
deferred (see the hardware plan).

| Target | Purpose |
| --- | --- |
| `blank` | Drives all 31 pixel-data pins low (WS28xx idle); safe first flash |
| `mapper` | Lights one of the 31 outputs dim red; the button steps to the next |
| `color-test` | Whole tree dim red, green, or blue; the button cycles the color |
| `demo` | Red/green/blue spiral bands; the button rotates them |
| `probe` | Reports activity and polarity on the three former radio pads |
| `receiver` | Two-channel Falcon receiver |

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
make probe
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

`make receiver` builds the RS-485 receiver firmware. It samples two MAX3485
`RXD` outputs at once: channel 1 on STC pin 27 / P3.6 (verticals 1–15) and
channel 2 on STC pin 29 / P4.1 (verticals 16–30). Each channel is one
510-pixel RGB Falcon output; this tree uses Falcon ports 29 and 30. The
firmware replays the frame in the confirmed clockwise order. Brightness is
set by the Falcon port; the firmware only scales a frame down if its
estimated draw would exceed the power supply budget.

Flash it only with each MAX3485 configured receive-only (`T` open, `EN` low or
open, `TXD` unconnected). Use `make flash-probe` first to confirm each pad
sees its own correctly polarized signal. The first valid capture emits
`frame: captured and replayed` on the UART. See `docs/receiver.md` for the
bring-up procedure, troubleshooting, and the sampling delay.
