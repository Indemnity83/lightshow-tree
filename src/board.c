#include "board.h"

/* STC8H GPIO ports and their mode registers.  00 = quasi, 01 = push-pull,
 * 10 = input-only, 11 = open-drain. */
__sfr __at (0x80) P0;
__sfr __at (0x90) P1;
__sfr __at (0xa0) P2;
__sfr __at (0xb0) P3;
__sfr __at (0xc0) P4;
__sfr __at (0xc8) P5;

__sfr __at (0x93) P0M1;
__sfr __at (0x94) P0M0;
__sfr __at (0x91) P1M1;
__sfr __at (0x92) P1M0;
__sfr __at (0x95) P2M1;
__sfr __at (0x96) P2M0;
__sfr __at (0xb1) P3M1;
__sfr __at (0xb2) P3M0;
__sfr __at (0xb3) P4M1;
__sfr __at (0xb4) P4M0;
__sfr __at (0xc9) P5M1;
__sfr __at (0xca) P5M0;

/* P3.5 is the factory button. It is active-low after the Q5 transistor. */
#define BUTTON_MASK 0x20U

void board_init_pixel_idle(void) {
    /* Set latches low before enabling output drivers: no startup high pulse. */
    P0 = 0x00;
    P1 &= (unsigned char)~PIXEL_P1_MASK;
    P2 = 0x00;
    P4 &= (unsigned char)~PIXEL_P4_MASK;
    P5 &= (unsigned char)~PIXEL_P5_MASK;

    P0M1 = 0x00;
    P0M0 = PIXEL_P0_MASK;
    P1M1 &= (unsigned char)~PIXEL_P1_MASK;
    P1M0 |= PIXEL_P1_MASK;
    P2M1 = 0x00;
    P2M0 = PIXEL_P2_MASK;
    P4M1 &= (unsigned char)~PIXEL_P4_MASK;
    P4M0 |= PIXEL_P4_MASK;
    P5M1 &= (unsigned char)~PIXEL_P5_MASK;
    P5M0 |= PIXEL_P5_MASK;

    /* Q5 is an open-collector button interface. Keep P3.5 in the STC's
     * quasi-bidirectional input mode with its weak pull-up enabled. */
    P3M1 &= (unsigned char)~BUTTON_MASK;
    P3M0 &= (unsigned char)~BUTTON_MASK;
    P3 |= BUTTON_MASK;
}

void board_init_falcon_input(void) {
    /* P3.6 / MCU pin 27 and P4.1 / MCU pin 29.  The MAX3485s actively drive
     * these signals, so use high-impedance inputs with no weak pull-up. */
    P3M1 |= FALCON_INPUT1_MASK;
    P3M0 &= (unsigned char)~FALCON_INPUT1_MASK;
    P4M1 |= FALCON_INPUT2_MASK;
    P4M0 &= (unsigned char)~FALCON_INPUT2_MASK;
}

unsigned char board_button_pressed(void) {
    return (P3 & BUTTON_MASK) == 0U;
}
