# Falcon differential receiver firmware

`make receiver` builds the end-to-end firmware for the repurposed factory
board. It takes two Falcon differential pixel outputs, each 510 pixels.

## Signal path

```text
Falcon port 29 (pair 1)            Falcon port 30 (pair 2)
  -> MAX3485 #1 A/B                  -> MAX3485 #2 A/B
  -> RXD                             -> RXD
  -> radio pad 3                     -> radio pad 1
  -> STC pin 27 / P3.6               -> STC pin 29 / P4.1
            \                          /
             -> interleaved capture buffer (2 x 1,530 RGB bytes)
             -> thirty factory pixel outputs
```

Falcon ports 29 and 30 are this tree's setup; any two outputs that share one
differential RJ45 work. Channel 1 carries verticals 1–15 and channel 2 carries
verticals 16–30.

Each MAX3485 must be configured receive-only: leave its `T` jumper open,
leave `TXD` unconnected, and hold `EN` low or leave it open if its on-board
pull-down is confirmed. On the purchased breakout, `TXD` was observed to carry
a copy of the received data, so a `TXD` wire to the tree board would feed one
channel's data onto another pad.

## Behavior

1. The firmware waits for a WS281x reset/latch interval on channel 1.
2. A tight assembly loop captures 510 RGB pixels from both channels at once.
   Bit timing comes from channel 1's rising edges; channel 2 is sampled two
   clocks after channel 1. This relies on the Falcon clocking its ports out
   together, which held on the first two-channel test: a chase decoded
   cleanly on all 30 verticals.
3. The interleaved capture is split into replay order. If the frame's
   estimated draw exceeds the power budget, it is scaled down to fit (see
   [Power limiting](#power-limiting)).
4. It replays 34 pixels to each of the thirty verticals in the proven physical
   clockwise order. The factory star output is untouched.

Capture and replay are sequential. A 510-pixel input frame is about 15 ms and
the 1,020-pixel replay about 31 ms, so with a 40 fps Falcon source the tree
is expected to replay every second or third frame (roughly 13–20 fps). It is
intentionally the simplest timing-safe baseline; a later bit-plane
parallel-output engine can increase the rate.

Until channel 2 is wired, P4.1 is an undriven high-impedance input and
verticals 16–30 show noise.

## Falcon configuration

- Two outputs on the same differential RJ45, each 510 RGB pixels.
- No null pixels, grouping, or zig-zag.
- A low test brightness in the Falcon configuration during bring-up, as a
  second safety limit on top of the firmware's power limiter.

## Bring-up procedure

1. Flash the input probe and confirm each pad sees its own signal:

   ```sh
   make flash-probe PORT=/dev/cu.usbserial-210
   make terminal PORT=/dev/cu.usbserial-210
   ```

   The probe prints one line per ~90 ms sampling pass for pins 27, 28, and 29.
   For pin 27 it also prints the longest unbroken low and high runs. A
   correctly polarized stream is mostly low (`high` around 10–15% of 65,535)
   with a long low run (the inter-frame gap, several thousand samples) and
   short high runs. Mostly high with a long high run means A and B are
   swapped. `rises 0` means nothing is arriving on that pin.

2. Flash the receiver:

   ```sh
   make flash-receiver PORT=/dev/cu.usbserial-210
   ```

3. Watch UART at 115200 baud after a power cycle. The banner lists both
   channels, and the first valid frame reports:

   ```text
   frame: captured and replayed
   ```

4. Use a Falcon RGB or single-color test pattern first, then a moving pattern.

## Troubleshooting

| Symptom | Likely cause |
| --- | --- |
| No `frame:` message | Channel 1 A/B swapped, or no signal on pad 3 — check with the probe |
| All lit pixels dim white | Sample point too early: every bit reads as a one |
| Wrong or flickering colors | Sample point marginal, or channel 2 not bit-aligned with channel 1 |
| Verticals 16–30 noise only | Channel 2 not wired, A/B swapped, or wrong Falcon port |

## Power limiting

Brightness is controlled in the Falcon, the same as every other prop: this
tree's ports are set to 40%. The firmware passes levels through unchanged and
only steps in when a frame's estimated draw would exceed the power budget
(`RX_POWER_BUDGET_MA`, default 1,250 mA, about 78% of the supply's 1.60 A
rating). That frame is then scaled down uniformly, so colors keep their
ratios, just far enough to fit.

This matters because the F16v4's test mode ignores port brightness. All of
the measurements below were taken with the ports at 40%, yet the tree
received full level 255.

The UART reports `power: limiting` when the limiter engages and
`power: within budget` when it releases.

| Case | Level in | Level out | Estimated 30 V draw |
| --- | --- | --- | --- |
| Show content, full white at 40% port brightness | 102 | 102 | about 1.06 A |
| Falcon test mode, full white | 255 | 120 | about 1.24 A |

Overrides for testing, without editing the source:

```sh
make flash-receiver PORT=/dev/cu.usbserial-210 POWER_BUDGET=1000
make flash-receiver PORT=/dev/cu.usbserial-210 BRIGHTNESS=100
```

`BRIGHTNESS` applies the old fixed cap (0–255 mapped to 0–n) before the
limiter; the default, 255, disables it.

### Calibration

Current measured at the 30 V output of the AC/DC supply, Falcon test mode
(unscaled level 255) with a fixed firmware cap:

| Firmware cap | Falcon pattern | 30 V current |
| --- | --- | --- |
| — | All black | 0.04 A |
| 31 | Full red | 0.17 A |
| 31 | Full green | 0.18 A |
| 31 | Full blue | 0.17 A |
| 31 | Full white | 0.40 A |
| 100 | Full white, held several minutes | 1.03–1.05 A |

At cap 100 the two heat-sinked parts of the factory 30 V to 5 V converter
were warm but not hot.

The limiter models draw as 0.04 A idle plus 1 A per 306,000 units of summed
component level, from the cap-100 measurement (1,020 × 3 × 100 = 306,000
for 1.0 A above idle). Between levels 31 and 100 the draw rose more slowly
than linearly, so extrapolating from 100 overestimates higher levels and the
limit errs on the safe side.

The deferred 45-pixel star bypasses the MCU and the limiter. At 40% it adds
about 0.04 A; in test mode about 0.1 A.

## Timing tuning

The capture loop distinguishes a WS281x zero from a one by sampling a fixed
number of clocks after channel 1's rising edge: after a zero pulse has ended
(about 11 clocks at 32 MHz) but before a one pulse has (about 22). The delay
is `RX_SAMPLE_DELAY` in `src/receiver.c`, emitted as that many single-clock
`nop`s with the assembler's `.rept`. The default, 13, is the middle of the
measured clean range below.

Cycle-counted sampling is the standard technique here: at 40 clocks per bit
there is no time for an interrupt per bit, and the STC8H has no peripheral
that decodes WS281x pulse widths on these pins. The 1T core runs the loop
deterministically; the only jitter is the 0–3 clocks the polling loop takes to
see the rising edge.

### Margin test

Build and flash a different delay without editing the source:

```sh
make flash-receiver PORT=/dev/cu.usbserial-210 SAMPLE_DELAY=10
```

Each value builds into its own `build/receiver-delayN/` directory. Plain
`make flash-receiver` returns to the default.

Use a Falcon test pattern with distinct colors (RGB or a chase). Step the
delay down until the tree turns whitish, then up until colors drop out or
darken, and set the default to the midpoint of the values that decode
cleanly.

Results on this tree (Falcon chase and RGB tests, both channels):

| Delay | Result |
| --- | --- |
| 7 | Every bit read as a one: dim white |
| 10 | Wrong colors on some channels: zeros misread |
| 12 | Clean |
| 14 | Clean |
| 16 | Flicker, no wrong colors |
| **13** | **Default: middle of the clean range** |

The clean window is narrow, about ±2 clocks around 13. The flicker at 16
(rather than wrong colors) suggests the upper limit comes from the end-of-byte
store running into the next bit, not from the one-pulse width. Shortening the
work after the sample would widen the window if a different Falcon timing
setting or pixel type ever needs it.
