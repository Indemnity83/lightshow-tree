#ifndef LIGHTSHOW_TREE_BOARD_H
#define LIGHTSHOW_TREE_BOARD_H

/*
 * Confirmed target geometry. The factory star is intentionally excluded from
 * the first firmware target and will be replaced with a separate coro star.
 */
#define TREE_VERTICAL_COUNT 30U
#define TREE_PIXELS_PER_VERTICAL 34U
#define TREE_PIXEL_COUNT (TREE_VERTICAL_COUNT * TREE_PIXELS_PER_VERTICAL)
#define TREE_FRAME_BYTES (TREE_PIXEL_COUNT * 3U)

/* Former radio pads, repurposed as MAX3485 receiver outputs:
 * channel 1 on pad 3 (MCU pin 27 / P3.6), channel 2 on pad 1 (pin 29 / P4.1). */
#define FALCON_INPUT1_MASK 0x40U
#define FALCON_INPUT2_MASK 0x02U

/* All 31 physical pixel data pins, arranged by MCU port bit. */
#define PIXEL_P0_MASK 0xffU
#define PIXEL_P1_MASK 0x3bU
#define PIXEL_P2_MASK 0xffU
#define PIXEL_P4_MASK 0xfdU
#define PIXEL_P5_MASK 0x1cU

void board_init_pixel_idle(void);
void board_init_falcon_input(void);
unsigned char board_button_pressed(void);

#endif
