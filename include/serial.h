#ifndef LIGHTSHOW_TREE_SERIAL_H
#define LIGHTSHOW_TREE_SERIAL_H

void serial_init(void);
void serial_puts(const char *text);
void serial_pin(unsigned char pin);
void serial_u16(unsigned int value);

#endif
