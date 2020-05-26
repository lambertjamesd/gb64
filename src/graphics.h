
#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "memory_map.h"

#define GB_SCREEN_W    160
#define GB_SCREEN_H    144

#define RENDER_TO_X 80
#define RENDER_TO_Y 48

#define TILE_ATTR_PALLETE       0x07
#define TILE_ATTR_VRAM_BANK     0x08
#define TILE_ATTR_H_FLIP        0x20
#define TILE_ATTR_V_FLIP        0x40
#define TILE_ATTR_PRIORITY      0x80

#define LCDC_BG_ENABLE          0x01
#define LCDC_OBJ_ENABLE         0x02
#define LCDC_OBJ_SIZE           0x04
#define LCDC_BG_TILE_MAP        0x08
#define LCDC_BG_TILE_DATA       0x10
#define LCDC_WIN_E              0x20
#define LCDC_WIN_TILE_MAP       0x40
#define LCDC_LCD_E              0x80

#define READ_PIXEL_INDEX(pixel, x) (x == 7) ? ((((pixel) >> 8) & 0x1) | (((pixel) << 1) & 0x2)) : ((((pixel) >> (15 - x)) & 0x1) | (((pixel) >> (6 - x)) & 0x2))

void renderPixelRow(
    struct Memory* memory,
    u16* memoryBuffer, 
    int row,
    int gbc
);

#endif