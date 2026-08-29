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

/* All 31 physical pixel data pins, arranged by MCU port bit. */
#define PIXEL_P0_MASK 0xffU
#define PIXEL_P1_MASK 0x3bU
#define PIXEL_P2_MASK 0xffU
#define PIXEL_P4_MASK 0xfdU
#define PIXEL_P5_MASK 0x1cU

void board_init_pixel_idle(void);
unsigned char board_button_pressed(void);

#endif
