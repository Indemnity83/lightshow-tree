#include "serial.h"

/* STC8 UART1 using Timer 2 as its baud generator.  At 32 MHz, 0xffbb gives
 * 115.9 kbaud (0.6% fast), within normal UART tolerance. */
__sfr __at (0x98) SCON;
__sfr __at (0x99) SBUF;
__sfr __at (0x8e) AUXR;
__sfr __at (0xd6) T2H;
__sfr __at (0xd7) T2L;
__sfr __at (0xb0) P3;
__sfr __at (0xb1) P3M1;
__sfr __at (0xb2) P3M0;
__sbit __at (0x99) TI;

static void serial_putc(unsigned char value) {
    while (!TI) { }
    TI = 0;
    SBUF = value;
}

void serial_init(void) {
    /* P3.1 / MCU pin 20 is UART TX; preserve all other P3 functions. */
    P3M1 &= 0xfdU;
    P3M0 |= 0x02U;
    P3 |= 0x02U;

    SCON = 0x50U;       /* UART mode 1, receiver enabled. */
    T2H = 0xffU;
    T2L = 0xbbU;
    AUXR |= 0x15U;      /* UART1 from Timer2, Timer2 1T, start Timer2. */
    TI = 1;
}

void serial_puts(const char *text) {
    while (*text != '\0') serial_putc((unsigned char)*text++);
}

void serial_pin(unsigned char pin) {
    unsigned char tens = (unsigned char)(pin / 10U);
    if (tens != 0U) serial_putc((unsigned char)('0' + tens));
    serial_putc((unsigned char)('0' + (pin % 10U)));
}
