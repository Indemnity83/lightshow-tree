#ifndef LIGHTSHOW_TREE_MAPPER_H
#define LIGHTSHOW_TREE_MAPPER_H

void mapper_init(void);
void mapper_service(void);

/* Send a 34-pixel RGB frame to one physical vertical.  vertical is in the
 * confirmed clockwise tree order, 0 through 29. */
void pixel_send_vertical_frame(unsigned char vertical,
                               const unsigned char *rgb);

#endif
