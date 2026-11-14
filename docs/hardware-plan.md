# Hardware plan: Falcon differential input

This is the first planned hardware implementation. It retains the Member's
Mark Lightshow Tree factory power board, STC8H8K64U, and thirty factory pixel
outputs. The factory 2.4 GHz radio is removed.

## Purchased modules

| Role | Module | Purchase reference |
| --- | --- | --- |
| Differential receiver at the tree | 3.3 V MAX3485 breakout with `A`, `B`, `GND`, `VCC`, `RXD`, `TXD`, and `EN` pads | [Amazon MAX3485 breakout](https://a.co/d/02lRfonH) |
| Cable termination / breakout at tree base | Dual parallel-RJ45 Cat5/6 breakout board | [Amazon RJ45 breakout](https://a.co/d/054dWxlo) |

The MAX3485 breakout is a practical wired proof-of-concept. A future compact
KiCad daughterboard may replace it. Its schematic should preserve the complete
receiver circuit below, rather than treating the transceiver as a bare IC.

## Future reproducible daughterboard

The production/daughterboard version should fit the former radio location. Its
host-interface symbol retains five available factory landing-pad nets, but
revision 1 connects only the three needed at run time: 3.3 V, ground, and STC
GPIO 27. It should contain:

| Ref. | Part / value | Connection / purpose |
| --- | --- | --- |
| U1 | MAX3485, SOIC-8 | 3.3 V RS-485 receiver/transceiver |
| RTERM | 120 Ω | A to B; populated because this board is the cable endpoint |
| RPU | 680 Ω | A to 3.3 V failsafe bias |
| RPD | 680 Ω | B to ground failsafe bias |
| REN | 1 kΩ | shared `DE`/`/RE` node to ground; default receive mode |
| C1 | 100 nF, X7R | directly from U1 VCC to ground |
| C2 | 1 µF, X7R | local 3.3 V bulk decoupling |
| J1 | 2-position pigtail terminal/pads | selected differential pair, A and B |
| H1 | custom five-net host interface, SOP-8 landing pattern | factory radio pads 1, 2, 3, 4, and 7 |

Tie `DE` and `/RE` together and pull them low through REN. Leave U1 `DI`
grounded or unconnected with a defined pull-down. The result is receive-only
by default; a future revision may expose the shared enable node on a test pad.

The purchased module appears to use 4.7 kΩ bias resistors. Those weak pulls
are sufficient as a convenience/default state while the Falcon is actively
driving the cable, but they only create about 42 mV across a 120 Ω terminator
at 3.3 V. The MAX3485's guaranteed differential threshold is ±200 mV, so the
rebuildable board uses 680 Ω pull-up/pull-down positions: they establish about
268 mV while drawing roughly 2.2 mA when the bus is undriven. The resistor
footprints may be marked optional if the Falcon source is later confirmed to
provide a stronger bias network.

## Wiring plan

```text
Falcon differential receiver port
  -> Cat5/6 cable
  -> base-mounted dual-RJ45 breakout
  -> one selected twisted pair up the tree
  -> MAX3485 A/B
  -> MAX3485 RXD
  -> factory radio pad 3 / STC pin 27 / P3.6
```

At the top of the tree, the MAX3485 breakout is wired as follows:

| MAX3485 breakout pad | Factory board connection | Purpose |
| --- | --- | --- |
| `VCC` | former radio pad 4 / 3.3 V | powers the receiver |
| `GND` | former radio pad 7 / ground | logic ground return |
| `RXD` | former radio pad 3 / STC pin 27 / P3.6 | received pixel waveform to firmware |
| `A`, `B` | selected twisted pair from base breakout | Falcon differential data |
| `TXD` | not connected | no transmitting is required |
| `EN` | ground, or open if the module pull-down is confirmed | receive-only mode |
| `T` solder jumper | leave open | do not force transmit mode |

The CAT-side ground terminal is not used. The MAX3485 receives its local power
and ground from the factory board, while the Cat5/6 cable carries only one
twisted data pair.

## Pair selection

Selection happens physically at the MAX3485 board: connect the desired pair
from the base breakout to `A` and `B`.

| Falcon logical output | Cat5/6 conductors (TIA-568B) |
| --- | --- |
| 1 | white/orange + orange |
| 2 | white/green + green |
| 3 | white/blue + blue |
| 4 | white/brown + brown |

Start with the pair's `+` conductor on `A` and its `-` conductor on `B`.
If the first hardware test does not decode cleanly, swap only A/B at the
MAX3485; leave the pair twisted together throughout the run.

## Power and safety limits

- The factory 30 V / 1.6 A supply remains responsible for the tree's 5 V
  pixel rail.
- The MAX3485 is powered from the existing 3.3 V logic rail, not from Cat5/6.
- The receiver firmware applies a 1/8 component-brightness limit as a default.
- Configure a low Falcon test brightness as an additional guard during initial
  bring-up.

## Planned 180° tree layout

For a front-facing 180° display configuration, use the 16 boundary-inclusive
strings in this physical order: `J21`, `J22`, …, `J35`, `J6`. `J21` and `J6`
are opposite sides of the original 360° tree, so this places a string at both
physical edges of the half-tree.
