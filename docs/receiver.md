# Falcon differential receiver firmware

`make receiver` builds the first end-to-end firmware for the repurposed
factory board.

## Signal path

```text
Falcon differential pair
  -> MAX3485 A/B
  -> MAX3485 RO / RXD
  -> STC8H8K64U pin 27 / P3.6
  -> capture buffer (1,530 RGB bytes from Falcon port 1)
  -> thirty factory pixel outputs
```

The MAX3485 must be configured receive-only: leave its `T` jumper open, leave
`TXD` unused, and hold `EN` low or leave it open if its on-board pull-down is
confirmed.

## First-pass behavior

1. The firmware waits for a WS281x reset/latch interval.
2. A tight assembly loop captures exactly 510 RGB pixels from P3.6. These are
   verticals 1–15; verticals 16–30 are sent black until a second receiver is
   added on Falcon port 2.
3. Every component is divided by eight to protect the factory power supply.
4. It replays 34 pixels to each of the thirty verticals in the proven physical
   clockwise order. The factory star output is untouched.

The input-frame and output-frame phases are sequential. A 510-pixel WS281x
input frame is about 15 ms and the 1,020-pixel replay about 31 ms, so with a
40 fps Falcon source this initial implementation is expected to replay every
other frame, near 20 frames per second. It is intentionally the simplest
timing-safe baseline; a later bit-plane parallel-output engine can increase
the rate.

## First hardware test

1. Set Falcon port 1 to 510 RGB pixels (no null pixels or grouping) and set a
   low test brightness in the Falcon configuration as an additional safety
   limit.
2. Connect Falcon pair 1 (white/orange + orange) to MAX3485 `A` and `B`; wire
   `VCC`, `GND`, and `RXD` to factory radio pads 4, 7, and 3 respectively.
3. Flash with:

   ```sh
   make flash-receiver PORT=/dev/cu.usbserial-210
   ```

4. Watch UART at 115200 baud. The first valid frame reports:

   ```text
   frame: captured and replayed
   ```

5. Start with a Falcon test pattern using a solid dim color, then a moving
   pattern. If the pixels remain dark or show unstable colors, first swap A/B.

## Timing tuning

The capture loop distinguishes a WS281x zero from a one by sampling P3.6 after
the rising edge and before a normal zero pulse has ended. The loop currently
contains seven explicitly visible `nop` instructions in each sampling site in
`src/receiver.c`.

That count is a first estimate for the 32 MHz MCU clock and must be verified on
the actual Falcon/MAX3485 combination. Add or remove one `nop` at **both**
sampling sites together if an oscilloscope or logic analyzer shows the sample
falling outside the gap between the zero and one high times.
