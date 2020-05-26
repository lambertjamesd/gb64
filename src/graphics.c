

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
    int row,
    int gbc
)
{
    int x;
    int offsetX;
    int windowX;
    int windowY;
    int tilemapRow;
    int tileInfo;
    u32 tileIndex;
    int tilemapIndex;
    u16* targetMemory;
    u16* pallete;
    u16 spriteRow;
    struct Tile* tilemapSource;
    int dataSelect;

    targetMemory = memoryBuffer + RENDER_TO_X + (row + RENDER_TO_Y) * SCREEN_WD;
    offsetX = -READ_REGISTER_DIRECT(memory, REG_SCX);
    windowY = (row - READ_REGISTER_DIRECT(memory, REG_SCY)) & 0xFF;

    tilemapRow = (windowY >> 3) * TILEMAP_W;

    tilemapRow += (READ_REGISTER_DIRECT(memory, REG_LCDC) & LCDC_BG_TILE_MAP) ? 1024 : 0;
    dataSelect = READ_REGISTER_DIRECT(memory, REG_LCDC) & LCDC_BG_TILE_DATA;

    if (!gbc)
    {
        pallete = (u16*)memory->misc.monochromePallete;
        tilemapSource = memory->vram.tiles;
        tileInfo = 0;
    }

    for (x = 0; x < GB_SCREEN_W;)
    {
        windowX = (x + offsetX) & 0xFF;
        tilemapIndex = tilemapRow + (windowX >> 3);

        if (gbc)
        {
            tileInfo = memory->vram.tilemap0Atts[tilemapIndex];
            pallete = (u16*)memory->misc.colorPalletes + ((tileInfo & TILE_ATTR_PALLETE) << 2);
            tilemapSource = (tileInfo & TILE_ATTR_VRAM_BANK) ? memory->vram.gbcTiles : memory->vram.tiles;
        }

        tileIndex = memory->vram.tilemap0[tilemapIndex];
        
        if (!dataSelect)
        {
            tileIndex = 256 + (char)tileIndex;
        }

        spriteRow = memory->vram.tiles[tileIndex].rows[
            (tileInfo & TILE_ATTR_V_FLIP) ?
                (7 - (windowY & 0x7)) :
                (windowY & 0x7)
        ];
        
        // A bit of a hack here
        // set the h flip flag to bit 3 and put the 
        // case range 8-15 to render the tile flipped
        switch ((windowX & 0x7) | ((tileInfo & TILE_ATTR_H_FLIP) >> 1))
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
                break;
            case 8:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 7);
            case 9:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 6);
            case 10:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 5);
            case 11:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 4);
            case 12:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 3);
            case 13:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 2);
            case 14:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 1);
            case 15:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 0);
                break;
        }
    }
}