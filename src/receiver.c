#include "board.h"
#include "mapper.h"
#include "receiver.h"
#include "serial.h"

__sbit __at (0xb6) FALCON_INPUT;

/*
 * Blind first pass for the MAX3485 receiver board.
 *
 * Input:  one RGB WS281x-style stream containing 15 * 34 = 510 pixels
 *         (Falcon port 1).  A second receiver will later carry verticals
 *         16-30; until then their half of the buffer stays black.
 * Output: the whole buffer, replayed one 34-pixel vertical at a time in the
 *         proven clockwise factory-wiring order.
 *
 * This is deliberately a polling receiver. P3.6 is sampled continuously in
 * a small assembly loop while a frame is present so UART activity cannot
 * disturb WS281x timing.
 * The MAX3485 delivers the Falcon pair at 3.3 V to P3.6.
 */

/* The factory 30 V / 1.6 A supply cannot support a full-white 1,020-pixel
 * frame. Clamp every component to 1/8 before replay (~12.5% maximum level). */
#define BRIGHTNESS_SHIFT 3U

/* Bytes received from Falcon port 1: verticals 1-15. */
#define CHANNEL_BYTES (TREE_FRAME_BYTES / 2U)

static __xdata unsigned char frame[TREE_FRAME_BYTES];
static unsigned char first_report;

/*
 * Capture an entire frame with no C code in the bit loop.
 *
 * At 32 MHz, a WS281x zero high is roughly 11 clocks and a one high roughly
 * 22 clocks. After detecting a rising edge, seven NOPs put the direct P3.6
 * sample between those two widths. A zero has fallen; a one remains high.
 *
 * The outer reset detector requires three complete 256-sample low passes, so
 * it cannot mistake an ordinary inter-bit low interval for a latch/reset.
 * The timing constants are intentionally local to this small assembly block:
 * an oscilloscope check may ask us to add or remove one NOP.
 */
static unsigned char capture_frame(void) __naked {
    __asm
        ; DPTR addresses the first 1,530 bytes of the XRAM capture buffer.
        mov     dptr,#_frame

        ; Synchronize to a >50 us low reset/latch period.
00001$:
        mov     r3,#0x03
00002$:
        mov     r4,#0x00
00003$:
        jb      _FALCON_INPUT,00001$
        djnz    r4,00003$
        djnz    r3,00002$

        ; Store 5 full 256-byte pages, then the remaining 250 bytes.
        mov     r7,#0x05
00004$:
        mov     r6,#0x00
00005$:
        mov     a,#0x00
        mov     r5,#0x08
00006$:
        jnb     _FALCON_INPUT,00006$
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        mov     c,_FALCON_INPUT
        rlc     a
00007$:
        jb      _FALCON_INPUT,00007$
        djnz    r5,00006$
        movx    @dptr,a
        inc     dptr
        djnz    r6,00005$
        djnz    r7,00004$

        mov     r6,#0xfa
00008$:
        mov     a,#0x00
        mov     r5,#0x08
00009$:
        jnb     _FALCON_INPUT,00009$
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        mov     c,_FALCON_INPUT
        rlc     a
00010$:
        jb      _FALCON_INPUT,00010$
        djnz    r5,00009$
        movx    @dptr,a
        inc     dptr
        djnz    r6,00008$

        mov     dpl,#0x01
        ret
    __endasm;
}

static void limit_brightness(void) {
    unsigned int index;

    for (index = 0U; index != CHANNEL_BYTES; ++index)
        frame[index] >>= BRIGHTNESS_SHIFT;
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
    serial_init();
    serial_puts("lightshow-tree Falcon receiver\r\n");
    serial_puts("input: P3.6 / MCU pin 27, 510 pixels\r\n");
    serial_puts("brightness: 1/8\r\n");
}

void receiver_service(void) {
    if (!capture_frame()) return;

    limit_brightness();
    replay_frame();

    /* One report confirms the frame path without flooding the UART. */
    if (first_report) {
        first_report = 0U;
        serial_puts("frame: captured and replayed\r\n");
    }
}
