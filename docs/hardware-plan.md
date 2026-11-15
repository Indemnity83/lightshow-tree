# Hardware plan: Falcon differential input

This is the first planned hardware implementation. It retains the Member's
Mark Lightshow Tree factory power board, STC8H8K64U, and thirty factory pixel
outputs. The factory 2.4 GHz radio is removed.

The tree takes two Falcon differential outputs of 510 pixels each, because a
single output cannot carry all 1,020 pixels. Each output has its own MAX3485
receiver, landing on a separate former radio pad.

## Purchased modules

| Role | Module | Purchase reference |
| --- | --- | --- |
| Differential receivers at the tree (two) | 3.3 V MAX3485 breakout with `A`, `B`, `GND`, `VCC`, `RXD`, `TXD`, and `EN` pads | [Amazon MAX3485 breakout](https://a.co/d/02lRfonH) |
| Cable termination / breakout at tree base | Dual parallel-RJ45 Cat5/6 breakout board | [Amazon RJ45 breakout](https://a.co/d/054dWxlo) |

The MAX3485 breakout is a practical wired proof-of-concept. A future compact
KiCad daughterboard may replace it. Its schematic should preserve the complete
receiver circuit below, rather than treating the transceiver as a bare IC.

## Future reproducible daughterboard

The production/daughterboard version should fit the former radio location. Its
host-interface symbol retains five available factory landing-pad nets. The
current KiCad revision is single-channel and connects only 3.3 V, ground, and
STC GPIO 27; the two-channel tree needs a second receiver (U1–C2 duplicated)
driving STC GPIO 29 / pad 1. Per channel it should contain:

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
  -> two twisted pairs up the tree
  -> pair 1 -> MAX3485 #1 A/B -> RXD -> radio pad 3 / STC pin 27 / P3.6
  -> pair 2 -> MAX3485 #2 A/B -> RXD -> radio pad 1 / STC pin 29 / P4.1
```

At the top of the tree, each MAX3485 breakout is wired as follows:

| MAX3485 breakout pad | Channel 1 (verticals 1–15) | Channel 2 (verticals 16–30) |
| --- | --- | --- |
| `VCC` | former radio pad 4 / 3.3 V | former radio pad 4 / 3.3 V |
| `GND` | former radio pad 7 / ground | former radio pad 7 / ground |
| `RXD` | former radio pad 3 / STC pin 27 / P3.6 | former radio pad 1 / STC pin 29 / P4.1 |
| `A`, `B` | pair 1 from base breakout | pair 2 from base breakout |
| `TXD` | not connected | not connected |
| `EN` | ground, or open if the module pull-down is confirmed | same |
| `T` solder jumper | leave open | leave open |

Leave `TXD` unconnected. On the purchased breakout it was observed to carry a
copy of the received data, so a `TXD` wire to the tree board would put one
channel's data on another pad.

Former radio pad 2 (STC pin 28 / P3.7) is a spare. The input probe read it as
constantly low, but nothing is visible on the trace and the firmware leaves it
in the STC8H's reset-default high-impedance mode, so it is most likely just
floating.

The CAT-side ground terminal is not used. The MAX3485s receive their local
power and ground from the factory board, while the Cat5/6 cable carries only
the twisted data pairs. Falcon's differential connector is understood to use
all four pairs for data, with no ground pair (not yet checked against the
Falcon manual). Running without a shared ground relies on the tree's 30 V
supply having a floating output.

## Pair selection

Falcon differential outputs come in groups of four per RJ45 (1–4, 5–8, …,
29–32). This tree uses outputs 29 and 30, the first two of their group.

| Output within the group | This tree | Cat5/6 conductors (TIA-568B) |
| --- | --- | --- |
| 1st | port 29, channel 1 | white/orange + orange |
| 2nd | port 30, channel 2 | white/green + green |
| 3rd | — | white/blue + blue |
| 4th | — | white/brown + brown |

Channel 1's initial A/B wiring was inverted on the first hardware test: the
input probe showed the line mostly high, with the inter-frame gap high.
Swapping A and B fixed it. Wire channel 2
the same way channel 1 ended up, and use the probe (`make flash-probe`) to
confirm polarity before flashing the receiver. Swap only A/B at the MAX3485;
leave each pair twisted together throughout the run.

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
