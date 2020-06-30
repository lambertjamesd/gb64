

#include <ultra64.h>
#include "graphics.h"
#include "../boot.h"
#include "debug_out.h"
#include "../memory.h"
#include "gameboy.h"

u8 gScreenBuffer[GB_SCREEN_W * GB_SCREEN_H];
u16 gScreenPalette[PALETTE_COUNT];

#define COPY_SCREEN_STRIP(blockY)                                        \
    gsDPLoadTextureTile(                                            \
        (int)gScreenBuffer + blockY * GB_SCREEN_W * GB_RENDER_STRIP_HEIGHT,                                         \
        G_IM_FMT_CI, G_IM_SIZ_8b,                                   \
        GB_SCREEN_W, GB_RENDER_STRIP_HEIGHT,                        \
        0, 0,                                  \
        GB_SCREEN_W - 1, GB_RENDER_STRIP_HEIGHT - 1,                \
        0,                                                          \
        G_TX_CLAMP, G_TX_CLAMP,                                       \
        G_TX_NOMASK, G_TX_NOMASK,                                   \
        G_TX_NOLOD, G_TX_NOLOD                                      \
    ),                                                              \
    gsSPTextureRectangle(                                           \
        RENDER_TO_X << 2, (RENDER_TO_Y + GB_RENDER_STRIP_HEIGHT * blockY) << 2,                       \
        (RENDER_TO_X + GB_SCREEN_W) << 2, (RENDER_TO_Y + GB_RENDER_STRIP_HEIGHT * (blockY + 1)) << 2, \
        G_TX_RENDERTILE,                                                                         \
        0, 0,                                                                                    \
        (1 << 10), (1 << 10)                                                                     \
    )

Gfx gDrawScreen[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetCombineMode(G_CC_BLENDRGBA, G_CC_BLENDRGBA),
    gsDPSetPrimColor(0, 0, 255, 255, 255, 255),
    gsDPLoadTLUT_pal256(gScreenPalette),
    gsDPSetTextureLUT(G_TT_RGBA16),

    COPY_SCREEN_STRIP(0),
    COPY_SCREEN_STRIP(1),
    COPY_SCREEN_STRIP(2),
    COPY_SCREEN_STRIP(3),
    COPY_SCREEN_STRIP(4),
    COPY_SCREEN_STRIP(5),
    COPY_SCREEN_STRIP(6),
    COPY_SCREEN_STRIP(7),
    COPY_SCREEN_STRIP(8),
    COPY_SCREEN_STRIP(9),
    COPY_SCREEN_STRIP(10),
    COPY_SCREEN_STRIP(11),

    gsSPEndDisplayList(),
};

#define WRITE_PIXEL(pixelIndex, x, targetMemory, spriteBuffer, maxX)    \
    if (spriteBuffer[x] && (!(spriteBuffer[x] & SPRITE_FLAGS_PRIORITY) || !pixelIndex)) \
        *targetMemory = (spriteBuffer[x] & ~SPRITE_FLAGS_PRIORITY) + OBJ_PALETTE_INDEX_START;                \
    else                                                                                        \
        *targetMemory = pixelIndex;                            \
    ++x;                                                                                        \
    ++targetMemory;                                                                             \
    if (x == maxX)                                                                       \
        break;

#define WRITE_SPRITE_PIXEL(spriteRow, paletteOffset, x, targetMemory, pixel)                                     \
    if (READ_PIXEL_INDEX(spriteRow, pixel) && *targetMemory == 0)                                                                                       \
        *targetMemory = READ_PIXEL_INDEX(spriteRow, pixel) + paletteOffset;                     \
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

    if (gbc)
    {
        int i;

        for (i = 0; i < PALETTE_COUNT; ++i)
        {
            gScreenPalette[i] = GBC_TO_N64_COLOR(gGameboy.memory.vram.colorPalettes[i]);
        }
    }
    else
    {
        memCopy(gScreenPalette, gGameboy.memory.vram.colorPalettes, sizeof(gScreenPalette));
    }

    state->gbc = gbc;
    state->row = 0;
    state->winY = 0;
}

void renderSprites(struct Memory* memory, struct GraphicsState* state)
{
    int x;
    int currentSpriteIndex;
    int renderedSprites = 0;
    int spriteHeight = (READ_REGISTER_DIRECT(memory, REG_LCDC) & LCDC_OBJ_SIZE) ? SPRITE_BASE_HEIGHT * 2 : SPRITE_BASE_HEIGHT;
    u8* targetMemory = state->spriteIndexBuffer;
    zeroMemory(targetMemory, GB_SCREEN_W);

    x = 0;
    currentSpriteIndex = 0;
    renderedSprites = 0;

    for (currentSpriteIndex = 0; currentSpriteIndex < state->spriteCount && renderedSprites < 10; ++currentSpriteIndex)
    {
        struct Sprite currentSprite = state->sortedSprites[currentSpriteIndex];
        int sourceX = 0;

        if (
            currentSprite.y - SPRITE_Y_OFFSET > state->row ||
            currentSprite.y - SPRITE_Y_OFFSET + spriteHeight <= state->row

        )
        {
            // sprite not on this row
            continue;
        }
        else
        {
            sourceX = x - ((int)currentSprite.x - SPRITE_WIDTH);
            ++renderedSprites;
        }

        if (sourceX > 0)
        {
            targetMemory -= sourceX;
            x -= sourceX;
            sourceX = 0;

            if (x < 0)
            {
                targetMemory -= x;
                sourceX -= x;
                x = 0;
            }
        }
        
        if (sourceX >= SPRITE_WIDTH)
        {
            continue;
        }
        
        while (x < GB_SCREEN_W && sourceX < 0)
        {
            *targetMemory = 0;
            ++targetMemory;
            ++sourceX;
            ++x;
        }

        u16 paletteIndex = state->gbc ? 
            (currentSprite.flags & SPRITE_FLAGS_GBC_PALETTE) << 2 : 
            ((currentSprite.flags & SPRITE_FLAGS_DMA_PALETTE) >> 2);

        if (currentSprite.flags & SPRITE_FLAGS_PRIORITY)
        {
            paletteIndex |= SPRITE_FLAGS_PRIORITY;
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
                WRITE_SPRITE_PIXEL(spriteRow, paletteIndex, x, targetMemory, 0);
            case 1:
                WRITE_SPRITE_PIXEL(spriteRow, paletteIndex, x, targetMemory, 1);
            case 2:
                WRITE_SPRITE_PIXEL(spriteRow, paletteIndex, x, targetMemory, 2);
            case 3:
                WRITE_SPRITE_PIXEL(spriteRow, paletteIndex, x, targetMemory, 3);
            case 4:
                WRITE_SPRITE_PIXEL(spriteRow, paletteIndex, x, targetMemory, 4);
            case 5:
                WRITE_SPRITE_PIXEL(spriteRow, paletteIndex, x, targetMemory, 5);
            case 6:
                WRITE_SPRITE_PIXEL(spriteRow, paletteIndex, x, targetMemory, 6);
            case 7:
                WRITE_SPRITE_PIXEL(spriteRow, paletteIndex, x, targetMemory, 7);
                break;
            case 8:
                WRITE_SPRITE_PIXEL(spriteRow, paletteIndex, x, targetMemory, 7);
            case 9:
                WRITE_SPRITE_PIXEL(spriteRow, paletteIndex, x, targetMemory, 6);
            case 10:
                WRITE_SPRITE_PIXEL(spriteRow, paletteIndex, x, targetMemory, 5);
            case 11:
                WRITE_SPRITE_PIXEL(spriteRow, paletteIndex, x, targetMemory, 4);
            case 12:
                WRITE_SPRITE_PIXEL(spriteRow, paletteIndex, x, targetMemory, 3);
            case 13:
                WRITE_SPRITE_PIXEL(spriteRow, paletteIndex, x, targetMemory, 2);
            case 14:
                WRITE_SPRITE_PIXEL(spriteRow, paletteIndex, x, targetMemory, 1);
            case 15:
                WRITE_SPRITE_PIXEL(spriteRow, paletteIndex, x, targetMemory, 0);
                break;
        }
    }

    while (x < GB_SCREEN_W)
    {
        *targetMemory = 0;
        ++targetMemory;
        ++x;
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
    int bgX;
    int bgY;
    int wX;
    int wY;
    int tilemapRow;
    int tileInfo;
    u32 tileIndex;
    int tilemapIndex;
    u8* targetMemory;
    u16 spriteRow;
    struct Tile* tileSource;
    int dataSelect;
    int lcdcReg;
    int maxX = GB_SCREEN_W;

    lcdcReg = READ_REGISTER_DIRECT(memory, REG_LCDC);

    // is sprites are disabled then this just clears the
    // sprite index memory
    renderSprites(memory, state);

    targetMemory = gScreenBuffer + state->row * GB_SCREEN_W;
    offsetX = READ_REGISTER_DIRECT(memory, REG_SCX);
    bgY = (state->row + READ_REGISTER_DIRECT(memory, REG_SCY)) & 0xFF;

    tilemapRow = (bgY >> 3) * TILEMAP_W;

    tilemapRow += (lcdcReg & LCDC_BG_TILE_MAP) ? 1024 : 0;
    dataSelect = lcdcReg & LCDC_BG_TILE_DATA;

    if (!state->gbc)
    {
        tileSource = memory->vram.tiles;
        tileInfo = 0;
    }
    
    wX = READ_REGISTER_DIRECT(memory, REG_WX) - WINDOW_X_OFFSET;
    wY = READ_REGISTER_DIRECT(memory, REG_WY);

    if (!(lcdcReg & LCDC_WIN_E) || 
        wX >= GB_SCREEN_W || 
        wY > state->row)
    {
        wX = GB_SCREEN_W;
    }
    else
    {
        maxX = wX;
    }

    for (x = 0; x < GB_SCREEN_W;)
    {
        if (x >= wX)
        {
            offsetX = x - wX;
            bgY = state->winY;
            tilemapRow = (bgY >> 3) * TILEMAP_W;
            tilemapRow += (READ_REGISTER_DIRECT(memory, REG_LCDC) & LCDC_WIN_TILE_MAP) ? 1024 : 0;
            wX = GB_SCREEN_W;
            ++state->winY;
        }

        bgX = (x + offsetX) & 0xFF;
        tilemapIndex = tilemapRow + (bgX >> 3);

        if (state->gbc)
        {
            tileInfo = memory->vram.tilemap0Atts[tilemapIndex];
            tileSource = (tileInfo & TILE_ATTR_VRAM_BANK) ? memory->vram.gbcTiles : memory->vram.tiles;
        }

        tileIndex = memory->vram.tilemap0[tilemapIndex];
        
        if (!dataSelect)
        {
            tileIndex = ((tileIndex + 0x80) & 0xFF) + 0x80;
        }

        spriteRow = tileSource[tileIndex].rows[
            (tileInfo & TILE_ATTR_V_FLIP) ?
                (7 - (bgY & 0x7)) :
                (bgY & 0x7)
        ];

        u8 pixelIndex;
        u8 pixelIndexOffset = (tileInfo & 0x7) << 2;
        
        // A bit of a hack here
        // set the h flip flag to bit 3 and put the 
        // case range 8-15 to render the tile flipped
        switch ((bgX & 0x7) | ((tileInfo & TILE_ATTR_H_FLIP) ? 8 : 0))
        {
            case 0:
                pixelIndex = READ_PIXEL_INDEX(spriteRow, 0) + pixelIndexOffset;
                WRITE_PIXEL(pixelIndex, x, targetMemory, state->spriteIndexBuffer, maxX);
            case 1:
                pixelIndex = READ_PIXEL_INDEX(spriteRow, 1) + pixelIndexOffset;
                WRITE_PIXEL(pixelIndex, x, targetMemory, state->spriteIndexBuffer, maxX);
            case 2:
                pixelIndex = READ_PIXEL_INDEX(spriteRow, 2) + pixelIndexOffset;
                WRITE_PIXEL(pixelIndex, x, targetMemory, state->spriteIndexBuffer, maxX);
            case 3:
                pixelIndex = READ_PIXEL_INDEX(spriteRow, 3) + pixelIndexOffset;
                WRITE_PIXEL(pixelIndex, x, targetMemory, state->spriteIndexBuffer, maxX);
            case 4:
                pixelIndex = READ_PIXEL_INDEX(spriteRow, 4) + pixelIndexOffset;
                WRITE_PIXEL(pixelIndex, x, targetMemory, state->spriteIndexBuffer, maxX);
            case 5:
                pixelIndex = READ_PIXEL_INDEX(spriteRow, 5) + pixelIndexOffset;
                WRITE_PIXEL(pixelIndex, x, targetMemory, state->spriteIndexBuffer, maxX);
            case 6:
                pixelIndex = READ_PIXEL_INDEX(spriteRow, 6) + pixelIndexOffset;
                WRITE_PIXEL(pixelIndex, x, targetMemory, state->spriteIndexBuffer, maxX);
            case 7:
                pixelIndex = READ_PIXEL_INDEX(spriteRow, 7) + pixelIndexOffset;
                WRITE_PIXEL(pixelIndex, x, targetMemory, state->spriteIndexBuffer, maxX);
                break;
            case 8:
                pixelIndex = READ_PIXEL_INDEX(spriteRow, 7) + pixelIndexOffset;
                WRITE_PIXEL(pixelIndex, x, targetMemory, state->spriteIndexBuffer, maxX);
            case 9:
                pixelIndex = READ_PIXEL_INDEX(spriteRow, 6) + pixelIndexOffset;
                WRITE_PIXEL(pixelIndex, x, targetMemory, state->spriteIndexBuffer, maxX);
            case 10:
                pixelIndex = READ_PIXEL_INDEX(spriteRow, 5) + pixelIndexOffset;
                WRITE_PIXEL(pixelIndex, x, targetMemory, state->spriteIndexBuffer, maxX);
            case 11:
                pixelIndex = READ_PIXEL_INDEX(spriteRow, 4) + pixelIndexOffset;
                WRITE_PIXEL(pixelIndex, x, targetMemory, state->spriteIndexBuffer, maxX);
            case 12:
                pixelIndex = READ_PIXEL_INDEX(spriteRow, 3) + pixelIndexOffset;
                WRITE_PIXEL(pixelIndex, x, targetMemory, state->spriteIndexBuffer, maxX);
            case 13:
                pixelIndex = READ_PIXEL_INDEX(spriteRow, 2) + pixelIndexOffset;
                WRITE_PIXEL(pixelIndex, x, targetMemory, state->spriteIndexBuffer, maxX);
            case 14:
                pixelIndex = READ_PIXEL_INDEX(spriteRow, 1) + pixelIndexOffset;
                WRITE_PIXEL(pixelIndex, x, targetMemory, state->spriteIndexBuffer, maxX);
            case 15:
                pixelIndex = READ_PIXEL_INDEX(spriteRow, 0) + pixelIndexOffset;
                WRITE_PIXEL(pixelIndex, x, targetMemory, state->spriteIndexBuffer, maxX);
                break;
        }
    }
}