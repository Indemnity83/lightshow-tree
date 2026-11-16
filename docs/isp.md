# STC UART ISP on macOS

## Wiring

Use the 3.3 V SparkFun Serial Basic. The tree board remains powered by its
normal 30 V supply; do not connect the adapter's VCC pin.

| Serial Basic | Tree header |
| --- | --- |
| TXO | J1-2 / T2 / STC pin 19 (RX) |
| RXI | J1-6 / T3 / STC pin 20 (TX) |
| GND | J1-5 / T4 |
| VCC, CTS, DTR | not connected |

The UART nets share the external flash `/CS` and `/HOLD` pins. This is expected
and should not conflict with the STC bootloader, because the flash does not
drive either shared net.

## Install tools

Install Apple command-line developer tools, SDCC, and `stc8prog`. The exact
package-manager commands are intentionally not prescribed here; use the
current installation guidance for your macOS setup.

`stc8prog` can be built from source:

```sh
xcode-select --install
git clone https://github.com/IOsetting/stc8prog.git
cd stc8prog
make
```

For this project, place that checkout at `tools/stc8prog/`; the Makefile uses
`tools/stc8prog/stc8prog` by default.

## Non-destructive connection test

1. Connect TX, RX, and ground with the tree unpowered.
2. Plug the Serial Basic into the Mac.
3. Find the serial device with `scripts/list-serial-ports.sh`.
4. Run `make detect PORT=/dev/cu.usbserial-XXXX`.
5. Apply or cycle the tree's normal 30 V power when the tool waits for the
   bootloader handshake.

`make detect` does not erase or program the MCU.

## Flashing

Only after a known-good firmware image has been built and the connection test
has succeeded:

```sh
make flash PORT=/dev/cu.usbserial-XXXX
```

This erases the factory MCU firmware before writing the project image.

To build and flash a specific target in one step, use `make flash-receiver` or
`make flash-probe` with the same `PORT=`. If the tool reports
`failed to detect chip`, the power cycle was missed; run the command again and
cycle power while it waits.
