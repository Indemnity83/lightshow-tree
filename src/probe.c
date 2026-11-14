#include "board.h"
#include "probe.h"
#include "serial.h"

__sfr __at (0xb0) P3;
__sfr __at (0xc0) P4;

/*
 * Input probe for MAX3485 bring-up.  Watches the three former radio pads:
 *
 *   pad 3: MCU pin 27 / P3.6  (the intended receiver input)
 *   pad 2: MCU pin 28 / P3.7
 *   pad 1: MCU pin 29 / P4.1
 *
 * Each pass samples 65,535 times in a plain C loop (well over one Falcon
 * frame), then reports for each pin:
 *
 *   high   samples that read high
 *   rises  low-to-high transitions seen (a C loop undercounts at 800 kHz,
 *          but zero means no signal is arriving on that pin)
 *
 * For pin 27 it also reports the longest unbroken low and high runs, in
 * samples.  A correct WS281x stream is mostly low with a long low run (the
 * reset gap); an inverted stream (A/B swapped) is mostly high with a long
 * high run.
 */

#define PAD3_MASK 0x40U     /* P3.6 */
#define PAD2_MASK 0x80U     /* P3.7 */
#define PAD1_MASK 0x02U     /* P4.1 */

static void serial_u16(unsigned int value) {
    char text[6];
    unsigned char length = 5U;

    text[5] = '\0';
    do {
        text[--length] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U);
    serial_puts(&text[length]);
}

static void report_pin(const char *label, unsigned int high,
                       unsigned int rises) {
    serial_puts(label);
    serial_puts(" high ");
    serial_u16(high);
    serial_puts(" rises ");
    serial_u16(rises);
}

void probe_init(void) {
    serial_init();
    serial_puts("lightshow-tree input probe\r\n");
    serial_puts("pins: 27=P3.6 28=P3.7 29=P4.1\r\n");
}

void probe_service(void) {
    unsigned int sample;
    unsigned int high27 = 0U, rises27 = 0U;
    unsigned int high28 = 0U, rises28 = 0U;
    unsigned int high29 = 0U, rises29 = 0U;
    unsigned int run = 0U, longest_low = 0U, longest_high = 0U;
    unsigned char last3 = P3, last4 = P4;
    unsigned char now3, now4;

    for (sample = 0U; sample != 0xffffU; ++sample) {
        now3 = P3;
        now4 = P4;

        if (now3 & PAD3_MASK) {
            ++high27;
            if (!(last3 & PAD3_MASK)) {
                ++rises27;
                if (run > longest_low) longest_low = run;
                run = 0U;
            }
        } else if (last3 & PAD3_MASK) {
            if (run > longest_high) longest_high = run;
            run = 0U;
        }
        ++run;

        if (now3 & PAD2_MASK) {
            ++high28;
            if (!(last3 & PAD2_MASK)) ++rises28;
        }
        if (now4 & PAD1_MASK) {
            ++high29;
            if (!(last4 & PAD1_MASK)) ++rises29;
        }

        last3 = now3;
        last4 = now4;
    }
    if (last3 & PAD3_MASK) {
        if (run > longest_high) longest_high = run;
    } else if (run > longest_low) {
        longest_low = run;
    }

    report_pin("27", high27, rises27);
    serial_puts(" lo ");
    serial_u16(longest_low);
    serial_puts(" hi ");
    serial_u16(longest_high);
    report_pin(" | 28", high28, rises28);
    report_pin(" | 29", high29, rises29);
    serial_puts("\r\n");
}
