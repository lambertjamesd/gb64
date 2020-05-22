

#include <ultra64.h>
#include "graphics.h"
#include "../boot.h"

void renderPixelRow(
    struct Memory* memory,
    u16* memoryBuffer, 
    int row
)
{
    int x;
    int offsetX;
    int windowX;
    int windowY;
    int tilemapRow;
    int tile;
    int tileRow;
    int xMask;
    int palleteIndex;
    u16* targetMemory;
    u16* pallete;
    u16 spriteRow;

    targetMemory = memoryBuffer + RENDER_TO_X + (row + RENDER_TO_Y) * SCREEN_WD;
    offsetX = -READ_REGISTER_DIRECT(memory, REG_SCX);
    windowY = (row - READ_REGISTER_DIRECT(memory, REG_SCY)) & 0xFF;

    tilemapRow = (windowY >> 3) * TILEMAP_W;

    pallete = (u16*)memory->misc.unused;

    for (x = 0; x < GB_SCREEN_W; ++x)
    {
        windowX = (x + offsetX) & 0xFF;
        tile = memory->vram.tilemap0[(windowX >> 3) + tilemapRow];
        palleteIndex = READ_PIXEL_INDEX(memory->vram.tiles[tile].rows[windowY & 0x7], windowX & 0x7);
        *targetMemory = pallete[palleteIndex];

        ++targetMemory;
    }
}