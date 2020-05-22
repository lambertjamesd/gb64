
#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "memory_map.h"

#define GB_SCREEN_W    160
#define GB_SCREEN_H    144

#define RENDER_TO_X 80
#define RENDER_TO_Y 48

#define READ_PIXEL_INDEX(pixel, x) ((((pixel) >> 8) & (0x80 >> x)) | (((pixel) & (0x80 >> x)) << 1))

void renderPixelRow(
    struct Memory* memory,
    u16* memoryBuffer, 
    int row
);

#endif