

#include <ultra64.h>
#include "graphics.h"
#include "../boot.h"

#define WRITE_PIXEL(spriteRow, pallete, x, targetMemory, pixel, spriteBuffer, spritePallete)    \
    if (spriteBuffer[x])                                                                        \
        *targetMemory = spritePallete[spriteBuffer[x] & ~SPRITE_FLAGS_PRIORITY];                \
    else                                                                                        \
        *targetMemory = pallete[READ_PIXEL_INDEX(spriteRow, pixel)];                            \
    ++x;                                                                                        \
    ++targetMemory;                                                                             \
    if (x == GB_SCREEN_W)                                                                       \
        break;

#define WRITE_SPRITE_PIXEL(spriteRow, palleteOffset, x, targetMemory, pixel)                    \
    if (READ_PIXEL_INDEX(spriteRow, pixel))                                                     \
        *targetMemory = READ_PIXEL_INDEX(spriteRow, pixel) + palleteOffset;                     \
    else                                                                                        \
        *targetMemory = 0;                                                                      \
    ++x;                                                                                        \
    ++targetMemory;                                                                             \
    if (x == GB_SCREEN_W)                                                                       \
        break;

int compareSprites(struct Sprite a, struct Sprite b)
{
    return a.x - b.x;
}

void sortSpritesRecursive(struct Sprite* spriteArray, struct Sprite* workingMemory, int count)
{
    if (count == 2)
    {
        // Only swap if not already sorted
        if (compareSprites(spriteArray[0], spriteArray[1]) > 0)
        {
            struct Sprite tmp = spriteArray[0];
            spriteArray[0] = spriteArray[1];
            spriteArray[1] = tmp;
        }
    }
    else if (count > 2)
    {
        int aIndex;
        int bIndex;
        int outputIndex;
        int midpoint = count / 2;
        sortSpritesRecursive(spriteArray, workingMemory, midpoint);
        sortSpritesRecursive(spriteArray + midpoint, workingMemory + midpoint, count - midpoint);

        outputIndex = 0;
        aIndex = 0;
        bIndex = midpoint;
        
        while (aIndex < midpoint && bIndex < count)
        {
            if (compareSprites(spriteArray[aIndex], spriteArray[bIndex]) <= 0)
            {
                workingMemory[outputIndex++] = spriteArray[aIndex++];
            }
            else
            {
                workingMemory[outputIndex++] = spriteArray[bIndex++];
            }
        }

        while (aIndex < midpoint)
        {
            workingMemory[outputIndex++] = spriteArray[aIndex++];
        }

        while (bIndex < count)
        {
            workingMemory[outputIndex++] = spriteArray[bIndex++];
        }

        for (outputIndex = 0; outputIndex < count; ++outputIndex)
        {
            spriteArray[outputIndex] = workingMemory[outputIndex];
        }
    }
}

void sortSprites(struct Sprite* array, int arrayLength)
{
    struct Sprite workingMemory[SPRITE_COUNT];
    sortSpritesRecursive(array, workingMemory, arrayLength);
}

void prepareSprites(struct Sprite* inputSprites, struct Sprite* sortedSprites, int *spriteCount)
{
    int currentOutput;
    int currentInput;

    currentOutput = 0;

    for (currentInput = 0; currentInput < SPRITE_COUNT; ++currentInput)
    {
        if (inputSprites[currentInput].y > 0 && inputSprites[currentInput].y < 160 &&
            inputSprites[currentInput].x < 168)
        {
            sortedSprites[currentOutput++] = inputSprites[currentInput];
        }
    }

    sortSprites(sortedSprites, currentOutput);
    *spriteCount = currentOutput;
}

void initGraphicsState(
    struct Memory* memory,
    struct GraphicsState* state,
    int gbc
)
{
    if (READ_REGISTER_DIRECT(memory, REG_LCDC) & LCDC_OBJ_ENABLE)
    {
        prepareSprites(memory->misc.sprites, state->sortedSprites, &state->spriteCount);
    }
    else
    {
        state->spriteCount = 0;
    }

    state->gbc = gbc;
    state->row = 0;
}

void renderSprites(struct Memory* memory, struct GraphicsState* state)
{
    int x;
    int currentSpriteIndex;
    int renderedSprites = 0;
    int spriteHeight = (READ_REGISTER_DIRECT(memory, REG_LCDC) & LCDC_OBJ_SIZE) ? SPRITE_BASE_HEIGHT * 2 : SPRITE_BASE_HEIGHT;
    u8* targetMemory = state->spriteIndexBuffer;

    x = 0;
    currentSpriteIndex = 0;
    renderedSprites = 0;

    while (x < GB_SCREEN_W)
    {
        struct Sprite currentSprite;
        int sourceX = GB_SCREEN_W;

        while (currentSpriteIndex < state->spriteCount && sourceX >= SPRITE_WIDTH)
        {
            currentSprite = state->sortedSprites[currentSpriteIndex];
            if (
                currentSprite.y - SPRITE_Y_OFFSET <= state->row && 
                currentSprite.y - SPRITE_Y_OFFSET + spriteHeight > state->row
            )
            {
                sourceX = x - (currentSprite.x - SPRITE_WIDTH);
                ++renderedSprites;
            }
            else
            {
                sourceX = GB_SCREEN_W;
            }
            ++currentSpriteIndex;
        } 

        while (x < GB_SCREEN_W && (sourceX < 0 || currentSpriteIndex == state->spriteCount || renderedSprites > 10))
        {
            *targetMemory = 0;
            ++targetMemory;
            ++sourceX;
            ++x;
        }

        if (x == GB_SCREEN_W)
        {
            break;
        }

        u16 palleteIndex = state->gbc ? 
            (currentSprite.flags & SPRITE_FLAGS_GBC_PALLETE) * 4 : 
            ((currentSprite.flags & SPRITE_FLAGS_DMA_PALLETE) >> 2);

        if (currentSprite.flags & SPRITE_FLAGS_PRIORITY)
        {
            palleteIndex |= SPRITE_FLAGS_PRIORITY;
        }

        int yIndex = state->row - (currentSprite.y - SPRITE_Y_OFFSET);

        if (currentSprite.flags & SPRITE_FLAGS_Y_FLIP)
        {
            yIndex = (spriteHeight - 1) - yIndex;
        }

        u16 spriteRow;
        struct Tile* tiles;

        if (state->gbc && (currentSprite.flags & SPRITE_FLAGS_VRAM_BANK))
        {
            tiles = memory->vram.gbcTiles;
        }
        else
        {
            tiles = memory->vram.tiles;
        }
        
        spriteRow = tiles[currentSprite.tile].rows[yIndex];

        switch (sourceX + ((currentSprite.flags & SPRITE_FLAGS_X_FLIP) ? 8 : 0))
        {
            case 0:
                WRITE_SPRITE_PIXEL(spriteRow, palleteIndex, x, targetMemory, 0);
            case 1:
                WRITE_SPRITE_PIXEL(spriteRow, palleteIndex, x, targetMemory, 1);
            case 2:
                WRITE_SPRITE_PIXEL(spriteRow, palleteIndex, x, targetMemory, 2);
            case 3:
                WRITE_SPRITE_PIXEL(spriteRow, palleteIndex, x, targetMemory, 3);
            case 4:
                WRITE_SPRITE_PIXEL(spriteRow, palleteIndex, x, targetMemory, 4);
            case 5:
                WRITE_SPRITE_PIXEL(spriteRow, palleteIndex, x, targetMemory, 5);
            case 6:
                WRITE_SPRITE_PIXEL(spriteRow, palleteIndex, x, targetMemory, 6);
            case 7:
                WRITE_SPRITE_PIXEL(spriteRow, palleteIndex, x, targetMemory, 7);
                break;
            case 8:
                WRITE_SPRITE_PIXEL(spriteRow, palleteIndex, x, targetMemory, 7);
            case 9:
                WRITE_SPRITE_PIXEL(spriteRow, palleteIndex, x, targetMemory, 6);
            case 10:
                WRITE_SPRITE_PIXEL(spriteRow, palleteIndex, x, targetMemory, 5);
            case 11:
                WRITE_SPRITE_PIXEL(spriteRow, palleteIndex, x, targetMemory, 4);
            case 12:
                WRITE_SPRITE_PIXEL(spriteRow, palleteIndex, x, targetMemory, 3);
            case 13:
                WRITE_SPRITE_PIXEL(spriteRow, palleteIndex, x, targetMemory, 2);
            case 14:
                WRITE_SPRITE_PIXEL(spriteRow, palleteIndex, x, targetMemory, 1);
            case 15:
                WRITE_SPRITE_PIXEL(spriteRow, palleteIndex, x, targetMemory, 0);
                break;
        }
    }
}

void renderPixelRow(
    struct Memory* memory,
    struct GraphicsState* state,
    u16* memoryBuffer
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
    struct Tile* tileSource;
    int dataSelect;
    int lcdcReg;

    lcdcReg = READ_REGISTER_DIRECT(memory, REG_LCDC);

    // is sprites are disabled then this just clears the
    // sprite index memory
    renderSprites(memory, state);

    targetMemory = memoryBuffer + RENDER_TO_X + (state->row + RENDER_TO_Y) * SCREEN_WD;
    offsetX = READ_REGISTER_DIRECT(memory, REG_SCX);
    windowY = (state->row + READ_REGISTER_DIRECT(memory, REG_SCY)) & 0xFF;

    tilemapRow = (windowY >> 3) * TILEMAP_W;

    tilemapRow += (lcdcReg & LCDC_BG_TILE_MAP) ? 1024 : 0;
    dataSelect = lcdcReg & LCDC_BG_TILE_DATA;

    if (!state->gbc)
    {
        pallete = (u16*)memory->vram.bgColorPalletes;
        tileSource = memory->vram.tiles;
        tileInfo = 0;
    }

    for (x = 0; x < GB_SCREEN_W;)
    {
        windowX = (x + offsetX) & 0xFF;
        tilemapIndex = tilemapRow + (windowX >> 3);

        if (state->gbc)
        {
            tileInfo = memory->vram.tilemap0Atts[tilemapIndex];
            pallete = (u16*)memory->vram.bgColorPalletes + ((tileInfo & TILE_ATTR_PALLETE) << 2);
            tileSource = (tileInfo & TILE_ATTR_VRAM_BANK) ? memory->vram.gbcTiles : memory->vram.tiles;
        }

        tileIndex = memory->vram.tilemap0[tilemapIndex];
        
        if (!dataSelect)
        {
            //tileIndex = ((tileIndex + 0x80) & 0xFF) + 0x80;
        }

        spriteRow = tileSource[tileIndex].rows[
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
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 0, state->spriteIndexBuffer, memory->vram.objColorPalletes);
            case 1:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 1, state->spriteIndexBuffer, memory->vram.objColorPalletes);
            case 2:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 2, state->spriteIndexBuffer, memory->vram.objColorPalletes);
            case 3:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 3, state->spriteIndexBuffer, memory->vram.objColorPalletes);
            case 4:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 4, state->spriteIndexBuffer, memory->vram.objColorPalletes);
            case 5:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 5, state->spriteIndexBuffer, memory->vram.objColorPalletes);
            case 6:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 6, state->spriteIndexBuffer, memory->vram.objColorPalletes);
            case 7:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 7, state->spriteIndexBuffer, memory->vram.objColorPalletes);
                break;
            case 8:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 7, state->spriteIndexBuffer, memory->vram.objColorPalletes);
            case 9:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 6, state->spriteIndexBuffer, memory->vram.objColorPalletes);
            case 10:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 5, state->spriteIndexBuffer, memory->vram.objColorPalletes);
            case 11:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 4, state->spriteIndexBuffer, memory->vram.objColorPalletes);
            case 12:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 3, state->spriteIndexBuffer, memory->vram.objColorPalletes);
            case 13:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 2, state->spriteIndexBuffer, memory->vram.objColorPalletes);
            case 14:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 1, state->spriteIndexBuffer, memory->vram.objColorPalletes);
            case 15:
                WRITE_PIXEL(spriteRow, pallete, x, targetMemory, 0, state->spriteIndexBuffer, memory->vram.objColorPalletes);
                break;
        }
    }
}