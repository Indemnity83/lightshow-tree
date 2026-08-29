#include "board.h"

#if defined(FIRMWARE_MAPPER) || defined(FIRMWARE_COLOR_TEST) || defined(FIRMWARE_DEMO)
#include "mapper.h"
#endif

void main(void) {
    /* First-flash image: every known data line is driven low.  This is an
     * idle WS28xx level, not a timed frame, so it cannot light pixels. */
    board_init_pixel_idle();

#if defined(FIRMWARE_MAPPER) || defined(FIRMWARE_COLOR_TEST) || defined(FIRMWARE_DEMO)
    mapper_init();
    for (;;) {
        mapper_service();
    }
#else
    for (;;) {
        /* Intentionally empty. Keep all outputs low and leave ISP pins alone. */
    }
#endif
}
