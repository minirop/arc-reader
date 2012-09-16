#ifndef WRITE_ETHORNELL_H
#define WRITE_ETHORNELL_H

#include <stdint.h>

/**
 * save a RGBA-array in a PNG file.
 */
int write_RGBA_to_png(uint16_t width, uint16_t height, uint8_t * array, const char * filename);

#endif

