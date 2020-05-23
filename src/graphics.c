

#include <ultra64.h>
#include "graphics.h"
#include "../boot.h"

#define WRITE_PIXEL(spriteRow, pallete, x, targetMemory, pixel)     \
    *targetMemory = pallete[READ_PIXEL_INDEX(spriteRow, pixel)];    \
    ++x;                                                            \
    ++targetMemory;                                                 \
    if (x == GB_SCREEN_W)                                           \
        break;

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
    u32 tileRow;
    int palleteIndex;
    int subLoop;
    u16* targetMemory;
    u16* pallete;
    u16 spriteRow;

    targetMemory = memoryBuffer + RENDER_TO_X + (row + RENDER_TO_Y) * SCREEN_WD;
    offsetX = -READ_REGISTER_DIRECT(memory, REG_SCX);
    windowY = (row - READ_REGISTER_DIRECT(memory, REG_SCY)) & 0xFF;

    tilemapRow = (windowY >> 3) * TILEMAP_W;

    pallete = (u16*)memory->misc.colorPalletes;

    for (x = 0; x < GB_SCREEN_W;)
    {
        windowX = (x + offsetX) & 0xFF;
        tileRow = memory->vram.tilemap0[tilemapRow + (windowX >> 3)];
        spriteRow = memory->vram.tiles[tileRow].rows[windowY & 0x7];
        
        switch (windowX & 0x7)
        {
            case 0:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 0);
            case 1:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 1);
            case 2:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 2);
            case 3:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 3);
            case 4:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 4);
            case 5:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 5);
            case 6:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 6);
            case 7:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 7);
        }
    }
}