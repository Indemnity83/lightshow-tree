#include "board.h"
#include "mapper.h"
#include "serial.h"

/*
 * Bring-up mapper.  It deliberately drives only one output at a time and uses
 * red=8/255 on every pixel in that vertical.  This keeps current negligible.
 *
 * The STC8H is operating at 32 MHz. The short pulse routines hold the line
 * high for approximately 0.3 us (zero) or 0.65 us (one); the function-call
 * gap remains low and is far below a WS281x reset interval.
 */

__sfr __at (0x80) P0;
__sfr __at (0x90) P1;
__sfr __at (0xa0) P2;
__sfr __at (0xc0) P4;
__sfr __at (0xc8) P5;

typedef void (*pulse_fn)(void);

/* C implementations compile to direct bit operations.  The timing-critical
 * high interval contains only fixed NOPs and the final port clear. */
#define PULSE_PAIR(tag, port, mask) \
static void z_##tag(void) { port |= (mask); __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; port &= (unsigned char)~(mask); } \
static void o_##tag(void) { port |= (mask); __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; __asm nop __endasm; port &= (unsigned char)~(mask); }

PULSE_PAIR(p1, P5, 0x08U) PULSE_PAIR(p2, P0, 0x20U)
PULSE_PAIR(p3, P0, 0x40U) PULSE_PAIR(p4, P0, 0x80U)
PULSE_PAIR(p5, P1, 0x01U) PULSE_PAIR(p6, P1, 0x02U)
PULSE_PAIR(p7, P4, 0x80U) PULSE_PAIR(p8, P1, 0x10U)
PULSE_PAIR(p9, P1, 0x20U) PULSE_PAIR(p12, P1, 0x08U)
PULSE_PAIR(p14, P5, 0x10U) PULSE_PAIR(p18, P4, 0x01U)
PULSE_PAIR(p30, P4, 0x04U) PULSE_PAIR(p31, P4, 0x08U)
PULSE_PAIR(p32, P4, 0x10U) PULSE_PAIR(p33, P2, 0x01U)
PULSE_PAIR(p34, P2, 0x02U) PULSE_PAIR(p35, P2, 0x04U)
PULSE_PAIR(p36, P2, 0x08U) PULSE_PAIR(p37, P2, 0x10U)
PULSE_PAIR(p38, P2, 0x20U) PULSE_PAIR(p39, P2, 0x40U)
PULSE_PAIR(p40, P2, 0x80U) PULSE_PAIR(p41, P4, 0x20U)
PULSE_PAIR(p42, P4, 0x40U) PULSE_PAIR(p43, P0, 0x01U)
PULSE_PAIR(p44, P0, 0x02U) PULSE_PAIR(p45, P0, 0x04U)
PULSE_PAIR(p46, P0, 0x08U) PULSE_PAIR(p47, P0, 0x10U)
PULSE_PAIR(p48, P5, 0x04U)

static const pulse_fn zero_pulse[31] = {
    z_p1,z_p2,z_p3,z_p4,z_p5,z_p6,z_p7,z_p8,z_p9,z_p12,z_p14,z_p18,
    z_p30,z_p31,z_p32,z_p33,z_p34,z_p35,z_p36,z_p37,z_p38,z_p39,z_p40,
    z_p41,z_p42,z_p43,z_p44,z_p45,z_p46,z_p47,z_p48
};
static const pulse_fn one_pulse[31] = {
    o_p1,o_p2,o_p3,o_p4,o_p5,o_p6,o_p7,o_p8,o_p9,o_p12,o_p14,o_p18,
    o_p30,o_p31,o_p32,o_p33,o_p34,o_p35,o_p36,o_p37,o_p38,o_p39,o_p40,
    o_p41,o_p42,o_p43,o_p44,o_p45,o_p46,o_p47,o_p48
};

static unsigned char selected;
static unsigned char was_pressed;
static unsigned char marker_pending;
static const unsigned char output_pin[31] = {
    1,2,3,4,5,6,7,8,9,12,14,18,30,31,32,33,34,35,36,37,38,39,40,41,42,
    43,44,45,46,47,48
};

/* Indexes into the 31-channel pulse tables. Index 9 is MCU pin 12 / star. */
static const unsigned char vertical_index[30] = {
    0,1,2,3,4,5,6,7,8,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,
    26,27,28,29,30
};

static void send_rgb_pixel(unsigned char red, unsigned char green,
                           unsigned char blue);

void pixel_send_vertical_frame(unsigned char vertical,
                               const unsigned char *rgb) {
    unsigned char pixel;

    selected = vertical_index[vertical];
    for (pixel = 0U; pixel != TREE_PIXELS_PER_VERTICAL; ++pixel) {
        send_rgb_pixel(rgb[0], rgb[1], rgb[2]);
        rgb += 3;
    }
}

#if defined(FIRMWARE_COLOR_TEST) || defined(FIRMWARE_DEMO)
static unsigned char color_index;
#endif

#ifdef FIRMWARE_DEMO
static unsigned char spiral_phase;
#endif

static void report_selected(void) {
    serial_puts("mapper: MCU pin ");
    serial_pin(output_pin[selected]);
    serial_puts("\r\n");
}

static void send_byte(unsigned char value) {
    unsigned char bit;
    for (bit = 0U; bit != 8U; ++bit) {
        if ((value & 0x80U) != 0U) one_pulse[selected]();
        else zero_pulse[selected]();
        value <<= 1;
    }
}

static void send_rgb_pixel(unsigned char red, unsigned char green,
                           unsigned char blue) {
    send_byte(red);
    send_byte(green);
    send_byte(blue);
}

static void send_rgb_frame(unsigned char red, unsigned char green,
                           unsigned char blue) {
    unsigned char pixel;
    for (pixel = 0U; pixel != TREE_PIXELS_PER_VERTICAL; ++pixel)
        send_rgb_pixel(red, green, blue);
}

static void send_black_frame(void) {
    unsigned char pixel;
    for (pixel = 0U; pixel != TREE_PIXELS_PER_VERTICAL; ++pixel) {
        send_byte(0x00U);
        send_byte(0x00U);
        send_byte(0x00U);
    }
}

#ifdef FIRMWARE_DEMO
static void send_spiral_frame(void) {
    unsigned char vertical;
    unsigned char pixel;
    for (vertical = 0U; vertical != 30U; ++vertical) {
        selected = vertical_index[vertical];
        for (pixel = 0U; pixel != TREE_PIXELS_PER_VERTICAL; ++pixel) {
            unsigned char band = (unsigned char)((vertical + (pixel / 3U) +
                                                   spiral_phase) % 3U);
            if (band == 0U) send_rgb_pixel(0x08U, 0x00U, 0x00U);
            else if (band == 1U) send_rgb_pixel(0x00U, 0x08U, 0x00U);
            else send_rgb_pixel(0x00U, 0x00U, 0x08U);
        }
    }
}
#endif

static void debounce_delay(void) {
    volatile unsigned int outer;
    volatile unsigned char inner;
    for (outer = 0U; outer != 1200U; ++outer)
        for (inner = 0U; inner != 20U; ++inner) { }
}

void mapper_init(void) {
    selected = 0U;
    was_pressed = 0U;
    marker_pending = 1U;
    serial_init();
#ifdef FIRMWARE_DEMO
    spiral_phase = 0U;
    serial_puts("lightshow-tree rainbow spiral\r\n");
    serial_puts("button: rotate\r\n");
#elif defined(FIRMWARE_COLOR_TEST)
    color_index = 0U;
    serial_puts("lightshow-tree color test\r\n");
    serial_puts("color: red\r\n");
#else
    serial_puts("lightshow-tree mapper\r\n");
    report_selected();
#endif
}

void mapper_service(void) {
    unsigned char pressed = board_button_pressed();
    if (pressed && !was_pressed) {
        debounce_delay();
        if (board_button_pressed()) {
#ifdef FIRMWARE_DEMO
            spiral_phase++;
            if (spiral_phase == 3U) spiral_phase = 0U;
            serial_puts("spiral: rotate\r\n");
            marker_pending = 1U;
#elif defined(FIRMWARE_COLOR_TEST)
            color_index++;
            if (color_index == 3U) color_index = 0U;
            serial_puts(color_index == 0U ? "color: red\r\n" :
                        color_index == 1U ? "color: green\r\n" :
                                            "color: blue\r\n");
            marker_pending = 1U;
#else
            send_black_frame();
            selected++;
            if (selected == 31U) selected = 0U;
            report_selected();
            marker_pending = 1U;
#endif
        }
    }
    was_pressed = pressed;
    if (marker_pending) {
#ifdef FIRMWARE_DEMO
        send_spiral_frame();
#elif defined(FIRMWARE_COLOR_TEST)
        unsigned char vertical;
        for (vertical = 0U; vertical != 30U; ++vertical) {
            selected = vertical_index[vertical];
            if (color_index == 0U) send_rgb_frame(0x08U, 0x00U, 0x00U);
            else if (color_index == 1U) send_rgb_frame(0x00U, 0x08U, 0x00U);
            else send_rgb_frame(0x00U, 0x00U, 0x08U);
        }
#else
        send_rgb_frame(0x08U, 0x00U, 0x00U);
#endif
        marker_pending = 0U;
    }
}
