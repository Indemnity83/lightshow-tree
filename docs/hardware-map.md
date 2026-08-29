# Hardware map

## Confirmed tree geometry

- 30 vertical strings
- 34 WS28xx-compatible pixels per vertical
- 1,020 pixels / 3,060 RGB bytes for the vertical tree
- The 45-pixel factory star is not part of the first firmware target.

## Power and outputs

- Factory input supply: 30 V DC, 1.6 A rating
- Pixel strings: 5 V, three wire (`+5 V`, data, ground)
- Logic rail: 3.3 V from the board's 1117 regulator
- The main MCU drives pixel data through `101` (100 ohm) resistor packs.

## MCU

Main MCU: STC8H8K64U, LQFP-48 package.

The table below uses *physical package pin numbers*, counted counter-clockwise
from the dot on the LQFP package. It records physical trace evidence first;
alternate peripheral labels are MCU capabilities, not proof of board use.

| Pin | MCU GPIO / dedicated function | Board use / tracing status |
| --- | --- | --- |
| 1 | P5.3 | Confirmed pixel-data output through a 100-ohm resistor |
| 2 | P0.5 | Confirmed pixel-data output through a 100-ohm resistor |
| 3 | P0.6 | Confirmed pixel-data output through a 100-ohm resistor |
| 4 | P0.7 | Confirmed pixel-data output through a 100-ohm resistor |
| 5 | P1.0 | Confirmed pixel-data output through a 100-ohm resistor |
| 6 | P1.1 | Confirmed pixel-data output through a 100-ohm resistor |
| 7 | P4.7 | Confirmed pixel-data output through a 100-ohm resistor |
| 8 | P1.4 / alternate SPI MISO | Confirmed pixel-data output through a 100-ohm resistor |
| 9 | P1.5 / alternate SPI SCLK | Confirmed pixel-data output through a 100-ohm resistor |
| 10 | P1.6 / XTAL0 | 32.000 MHz external crystal/resonator connection |
| 11 | P1.7 / XTAL1 | 32.000 MHz external crystal/resonator connection |
| 12 | P1.3 / alternate SPI MOSI | Confirmed pixel-data output through a 100-ohm resistor |
| 13 | UCap | Required capacitor C7 to ground for the MCU's internal USB supply/regulator; leave in place |
| 14 | P5.4 / NRST | Confirmed pixel-data output through a 100-ohm resistor; reset-capable GPIO |
| 15 | VCC | 3.3 V logic supply |
| 16 | ADC_VRef+ | Grounded on this board; do not use ADC functions |
| 17 | GND | Ground |
| 18 | P4.0 | Confirmed pixel-data output through a 100-ohm resistor |
| 19 | P3.0 / UART RX | T2 and J1-2; also factory-flash `/CS` |
| 20 | P3.1 / UART TX | T3 and J1-6; reported continuity to flash pin 7 (`/HOLD`) |
| 21 | P3.2 / SPI4 SCLK | Factory-flash clock; J1-1 / flash pin 6 (trace direction consistent) |
| 22 | P3.3 / SPI4 MISO | Factory-flash data-out; J1-4 / flash pin 2 (trace direction consistent) |
| 23 | P3.4 / SPI4 MOSI | Factory-flash data-in; J1-3 / flash pin 5 (trace direction consistent) |
| 24 | P5.0 | NC (no board connection) |
| 25 | P5.1 | NC (no board connection) |
| 26 | P3.5 | Factory physical-button input through Q5 (`J3Y`) |
| 27 | P3.6 | PAN2110 radio pin 3, `SDA/DATA` (bidirectional serial data) |
| 28 | P3.7 | PAN2110 radio pin 2, `SCL/SCK` (serial clock) |
| 29 | P4.1 | PAN2110 radio pin 1, `CSN` (active-low chip select) |
| 30 | P4.2 | Confirmed pixel-data output through a 100-ohm resistor |
| 31 | P4.3 | Confirmed pixel-data output through a 100-ohm resistor |
| 32 | P4.4 | Confirmed pixel-data output through a 100-ohm resistor |
| 33 | P2.0 | Confirmed pixel-data output through a 100-ohm resistor |
| 34 | P2.1 | Confirmed pixel-data output through a 100-ohm resistor |
| 35 | P2.2 | Confirmed pixel-data output through a 100-ohm resistor |
| 36 | P2.3 | Confirmed pixel-data output through a 100-ohm resistor |
| 37 | P2.4 | Confirmed pixel-data output through a 100-ohm resistor |
| 38 | P2.5 | Confirmed pixel-data output through a 100-ohm resistor |
| 39 | P2.6 | Confirmed pixel-data output through a 100-ohm resistor |
| 40 | P2.7 | Confirmed pixel-data output through a 100-ohm resistor |
| 41 | P4.5 | Confirmed pixel-data output through a 100-ohm resistor |
| 42 | P4.6 | Confirmed pixel-data output through a 100-ohm resistor |
| 43 | P0.0 | Confirmed pixel-data output through a 100-ohm resistor |
| 44 | P0.1 | Confirmed pixel-data output through a 100-ohm resistor |
| 45 | P0.2 | Confirmed pixel-data output through a 100-ohm resistor |
| 46 | P0.3 | Confirmed pixel-data output through a 100-ohm resistor |
| 47 | P0.4 | Confirmed pixel-data output through a 100-ohm resistor |
| 48 | P5.2 | Confirmed pixel-data output through a 100-ohm resistor |

### Pixel-output mapping status

Confirmed pixel-output pins: `1–9`, `12`, `14`, `18`, and `30–48`. This is
exactly 31 MCU output pins: 9 + 3 + 19. It matches the 30 vertical strings plus
the factory star. Pins 27–29 are the factory radio interface, pin 26 is the
physical-button input, and pin 25 has no observed connection.

### Mapper validation progress

- MCU pin 12: factory-star output.
- Vertical strings, in physical clockwise/circular order: MCU pins `1–9`,
  `14`, `18`, and `30–48`. This is exactly 30 verticals.
- MCU pin 26 / P3.5: confirmed active-low physical-button input. Q5 is an
  open-collector stage; the MCU must keep its weak pull-up enabled.
- Pixel byte order: RGB (a `00 08 00` test frame illuminated green).

Confirm every remaining candidate by measuring from the *MCU-side* of each
100-ohm resistor channel to the candidate MCU pins, with board power removed.
This avoids probing the fine-pitch MCU leads directly.

## External devices

- PAN2110P0AA: 2.4 GHz factory remote-control radio with PCB trace antenna
- 25TQ256-style device: external SPI NOR flash, likely factory effect storage

## J1 flash-service header

J1 pin 1 is adjacent to the `J1` silkscreen label.

| J1 pin | Flash pin | Function / board net |
| --- | --- | --- |
| 2 | 1 | `/CS`; STC pin 19 / UART RX |
| 4 | 2 | `DO` / MISO; STC pin 22 |
| NC | 3 | `/WP`; tied to 3.3 V |
| 5 | 4 | Ground |
| 3 | 5 | `DI` / MOSI; STC pin 23 |
| 1 | 6 | `CLK`; STC pin 21 |
| 6 | 7 | `/HOLD`; expected STC pin 20 / UART TX |
| NC | 8 | `VCC`; 3.3 V |

### J1 access for STC UART ISP

J1 is not a dedicated MCU-programming header, but three of its nets provide a
usable shared UART connection. Power the board normally from its own supply;
do **not** connect the USB-UART adapter's VCC pin.

| USB-UART adapter signal | J1 pin | STC connection | Notes |
| --- | --- | --- | --- |
| `TXO` | 2 | Pin 19, `P3.0` / UART RX | Shared with flash `/CS` |
| `RXI` | 6 | Pin 20, `P3.1` / UART TX | Shared with flash `/HOLD` |
| `GND` | 5 | Ground | Required common reference |
| 

Shared nets:

- J1-2 = T2 = STC pin 19 (`P3.0`, UART RX) = flash `/CS`
- J1-6 = T3 = STC pin 20 (`P3.1`, UART TX) = flash `/HOLD`

## UART test pads

The unnamed pads are numbered from the MCU outward.

| Pad | Net |
| --- | --- |
| T1 | 3.3 V / STC pin 15 |
| T2 | STC pin 19, UART RX |
| T3 | STC pin 20, UART TX |
| T4 | ground |

## Open items

- Identify the exact 30 output pin-to-string mapping.
- Confirm all radio-connected MCU pins before isolation.
- Prototype and validate a fast 3.3 V RS-485 receiver input stage.

## Factory off behavior

There is no identified MCU-controlled power-enable path. The factory remote and
physical-button “off” behavior is expected to send black (`R=G=B=0`) data to
all pixel outputs while the 30 V input, 5 V buck converter, radio, and MCU
remain powered.
