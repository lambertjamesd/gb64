
#include "sprite.h"
#include "assert.h"

#define DL_CHUNK_SIZE       16
#define SPRITE_GL_LENGTH    2098

Gfx* gLayerSetup[MAX_LAYER_COUNT];

u32 gCurrentSpriteColors[MAX_LAYER_COUNT];
Gfx gSpriteDisplayList[SPRITE_GL_LENGTH];
Gfx* gLayerDL[MAX_LAYER_COUNT];
Gfx* gCurrentLayerDL[MAX_LAYER_COUNT];
Gfx* gLayerChunk[MAX_LAYER_COUNT];
Gfx* gSpriteNextChunk;

Gfx* allocateSpriteChunk()
{
    Gfx* result = gSpriteNextChunk;

    gSpriteNextChunk += DL_CHUNK_SIZE;
    assert(gSpriteNextChunk <= gSpriteDisplayList + SPRITE_GL_LENGTH);

    return result;
}

void writeDL(int layer, Gfx* src, int count)
{
    while (count)
    {
        Gfx* current = gCurrentLayerDL[layer];
        int capacity = DL_CHUNK_SIZE + gLayerChunk[layer] - current;

        if (!current || capacity == 1)
        {
            Gfx* next = allocateSpriteChunk();

            if (current)
            {
                // check if the next chunk is adjacent in memory
                if (current + 1 == next)
                {
                    *current++ = *src++;
                    --count;
                    --capacity;
                }
                else
                {
                    gSPBranchList(current++, next);
                }
            }
            else
            {
                gLayerDL[layer] = next;
            }

            gLayerChunk[layer] = next;
            gCurrentLayerDL[layer] = next;
            current = next;
        }

        while (count && capacity > 1)
        {
            *current++ = *src++;
            --count;
            --capacity;
        }

        gCurrentLayerDL[layer] = current;
    }
}

void setLayerGraphics(int layer, Gfx* graphics)
{
    gLayerSetup[layer] = graphics;
}

void spriteDraw(int layer, int x, int y, int w, int h, int sx, int sy, int sw, int sh)
{
    Gfx workingMem[4];
    Gfx* curr = workingMem;

    gSPTextureRectangle(
        curr++,
        x << 2, 
        y << 2,
        (x + (w << sw)) << 2,
        (y + (h << sh)) << 2,
        G_TX_RENDERTILE,
        sx << 5, sy << 5,
        0x400 >> sw,
        0x400 >> sh
    );

    writeDL(layer, workingMem, curr - workingMem);
}

void spriteDrawTile(int layer, int x, int y, int w, int h, struct SpriteTile tile)
{
    Gfx workingMem[4];
    Gfx* curr = workingMem;

    gSPTextureRectangle(
        curr++,
        x << 2, 
        y << 2,
        (x + w) << 2,
        (y + h) << 2,
        G_TX_RENDERTILE,
        tile.x << 5, tile.y << 5,
        (tile.w << 10) / w,
        (tile.h << 10) / h
    );

    writeDL(layer, workingMem, curr - workingMem);
}

void spriteSetColor(int layer, u8 r, u8 g, u8 b, u8 a)
{
    int key = (r << 24) | (g << 16) | (b << 8) | (a << 0);

    if (key != gCurrentSpriteColors[layer])
    {
        Gfx workingMem;
        Gfx* curr = &workingMem;
        gDPSetEnvColor(curr++, r, g, b, a);
        writeDL(layer, &workingMem, curr - &workingMem);
        gCurrentSpriteColors[layer] = key;
    }
}

void initSprites()
{
    for (int i = 0; i < MAX_LAYER_COUNT; ++i)
    {
        gLayerDL[i] = NULL;
        gCurrentLayerDL[i] = NULL;
        gLayerChunk[i] = NULL;
        gCurrentSpriteColors[i] = ~0;
    }

    gSpriteNextChunk = gSpriteDisplayList;
}

void finishSprites(Gfx** out)
{
    Gfx* curr = *out;

    for (int i = 0; i < MAX_LAYER_COUNT; ++i)
    {
        if (gLayerDL[i] && gLayerSetup[i])
        {
            gSPEndDisplayList(gCurrentLayerDL[i]++);
            gSPDisplayList(curr++, gLayerSetup[i]);
            gSPDisplayList(curr++, gLayerDL[i]);
        }
    }

    osWritebackDCache(gSpriteDisplayList, sizeof(gSpriteDisplayList));

    *out = curr;
}
