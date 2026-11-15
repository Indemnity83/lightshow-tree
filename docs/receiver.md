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
   together; the first two-channel test confirms it.
3. The interleaved capture is split into replay order and every component is
   divided by eight to protect the factory power supply.
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
  second safety limit on top of the firmware's 1/8 limit.

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

## Timing tuning

The capture loop distinguishes a WS281x zero from a one by sampling after the
rising edge, after a zero pulse has ended but before a one pulse has. With
seven `nop`s every bit read as a one (dim white); twelve decoded the Falcon RGB
test pattern correctly on the first hardware test. The `nop`s are in the
single sampling site in `capture_frame` in `src/receiver.c`.

Twelve has not yet been margin-tested. Building with ten and with fourteen and
confirming that both still decode cleanly would show that twelve is centered.
An oscilloscope or logic analyzer on the RXD outputs gives the actual zero and
one high times directly.
