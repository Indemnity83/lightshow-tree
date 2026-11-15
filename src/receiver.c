#include "board.h"
#include "mapper.h"
#include "receiver.h"
#include "serial.h"

__sbit __at (0xb6) FALCON_INPUT1;
__sbit __at (0xc1) FALCON_INPUT2;

/*
 * Two-channel receiver for the MAX3485 boards.
 *
 * Input:  two RGB WS281x-style streams, each 15 * 34 = 510 pixels.
 *         Channel 1 (radio pad 3 / P3.6) carries verticals 1-15.
 *         Channel 2 (radio pad 1 / P4.1) carries verticals 16-30.
 * Output: all 30 verticals, replayed one 34-pixel vertical at a time in the
 *         proven clockwise factory-wiring order.
 *
 * This is deliberately a polling receiver. Both inputs are sampled in a small
 * assembly loop while a frame is present so UART activity cannot disturb
 * WS281x timing.  Bit timing comes from channel 1 alone: the Falcon clocks
 * its ports out together, so channel 2 is sampled at the same moment.
 */

/* The factory 30 V / 1.6 A supply cannot support a full-white 1,020-pixel
 * frame, so every component is scaled to 0..RX_BRIGHTNESS_LIMIT before
 * replay.  Measured at the 30 V input: 0.04 A idle plus 0.36 A for full white
 * at level 31, roughly linear.  100 predicts ~1.20 A full white (75% of the
 * supply rating).  See docs/receiver.md.  Override with `make flash-receiver BRIGHTNESS=n`. */
#ifndef RX_BRIGHTNESS_LIMIT
#define RX_BRIGHTNESS_LIMIT 100
#endif

#define STRINGIFY(x) #x
#define STRINGIFY_VALUE(x) STRINGIFY(x)

/* Clocks of delay between seeing a channel 1 rising edge and sampling.
 * Margin-tested on the tree: 10 and below misread zeros, 12-14 decode
 * cleanly, 16 flickers.  13 is the middle of the clean range.  Override with
 * `make flash-receiver SAMPLE_DELAY=n`; see docs/receiver.md. */
#ifndef RX_SAMPLE_DELAY
#define RX_SAMPLE_DELAY 13
#endif

/* Bytes received on each channel: 15 verticals. */
#define CHANNEL_BYTES (TREE_FRAME_BYTES / 2U)

/* Raw capture, channel bytes interleaved: ch1, ch2, ch1, ch2, ... */
static __xdata unsigned char capture[TREE_FRAME_BYTES];
/* Replay order: channel 1's 1,530 bytes, then channel 2's. */
static __xdata unsigned char frame[TREE_FRAME_BYTES];
/* Input level -> limited output level, built once at boot. */
static __xdata unsigned char brightness[256];
static unsigned char first_report;

/*
 * Capture an entire frame with no C code in the bit loop.
 *
 * At 32 MHz, a WS281x zero high is roughly 11 clocks and a one high roughly
 * 22 clocks, in a 40-clock bit. After detecting a channel 1 rising edge,
 * RX_SAMPLE_DELAY single-clock NOPs put the P3.6 sample between those two
 * widths; P4.1 is sampled two clocks later. A zero has fallen; a one remains
 * high.
 *
 * The byte store runs straight after the eighth sample, while the loop would
 * otherwise only be waiting for the falling edge, so it finishes before the
 * next bit's rising edge. 1,530 byte pairs are exactly 6 * 255, so the only
 * extra work is a counter reload every 255 pairs.
 *
 * The outer reset detector requires three complete 256-sample low passes, so
 * it cannot mistake an ordinary inter-bit low interval for a latch/reset.
 */
static unsigned char capture_frame(void) __naked {
    __asm
        ; DPTR addresses the interleaved 3,060-byte XRAM capture buffer.
        mov     dptr,#_capture

        ; Synchronize to a >50 us low reset/latch period on channel 1.
00001$:
        mov     r3,#0x03
00002$:
        mov     r4,#0x00
00003$:
        jb      _FALCON_INPUT1,00001$
        djnz    r4,00003$
        djnz    r3,00002$

        ; r7 pages of r6 byte pairs; r5 bits per byte.
        ; a collects channel 1, r2 collects channel 2.
        mov     r7,#0x06
        mov     r6,#0xff
        mov     r5,#0x08
00004$:
        jb      _FALCON_INPUT1,00004$
00005$:
        jnb     _FALCON_INPUT1,00005$
        .rept   RX_SAMPLE_DELAY
        nop
        .endm
        mov     c,_FALCON_INPUT1
        rlc     a
        mov     c,_FALCON_INPUT2
        xch     a,r2
        rlc     a
        xch     a,r2
        djnz    r5,00004$

        movx    @dptr,a
        inc     dptr
        xch     a,r2
        movx    @dptr,a
        inc     dptr
        mov     r5,#0x08
        djnz    r6,00004$
        mov     r6,#0xff
        djnz    r7,00004$

        mov     dpl,#0x01
        ret
    __endasm;
}

static void build_brightness_table(void) {
    unsigned int level;

    for (level = 0U; level != 256U; ++level)
        brightness[level] = (unsigned char)
            ((level * RX_BRIGHTNESS_LIMIT + 127U) / 255U);
}

/* Split the interleaved capture into replay order, applying the brightness
 * limit on the way. */
static void unpack_frame(void) {
    const __xdata unsigned char *source = capture;
    __xdata unsigned char *channel1 = frame;
    __xdata unsigned char *channel2 = frame + CHANNEL_BYTES;
    unsigned int count;

    for (count = CHANNEL_BYTES; count != 0U; --count) {
        *channel1++ = brightness[*source++];
        *channel2++ = brightness[*source++];
    }
}

static void replay_frame(void) {
    unsigned char vertical;
    const unsigned char *pixels = frame;

    for (vertical = 0U; vertical != TREE_VERTICAL_COUNT; ++vertical) {
        pixel_send_vertical_frame(vertical, pixels);
        pixels += TREE_PIXELS_PER_VERTICAL * 3U;
    }
}

void receiver_init(void) {
    first_report = 1U;
    build_brightness_table();
    serial_init();
    serial_puts("lightshow-tree Falcon receiver\r\n");
    serial_puts("ch1: P3.6 / MCU pin 27, verticals 1-15\r\n");
    serial_puts("ch2: P4.1 / MCU pin 29, verticals 16-30\r\n");
    serial_puts("brightness limit: "
                STRINGIFY_VALUE(RX_BRIGHTNESS_LIMIT) "/255\r\n");
}

void receiver_service(void) {
    if (!capture_frame()) return;

    unpack_frame();
    replay_frame();

    /* One report confirms the frame path without flooding the UART. */
    if (first_report) {
        first_report = 0U;
        serial_puts("frame: captured and replayed\r\n");
    }
}
