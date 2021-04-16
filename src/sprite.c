
#include "sprite.h"
#include "assert.h"

#define DL_CHUNK_SIZE       16
#define SPRITE_GL_LENGTH    512

Gfx* gLayerSetup[MAX_LAYER_COUNT];

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

void drawSprite(int layer, int x, int y, int w, int h, int sx, int sy, int sw, int sh)
{
    Gfx workingMem[4];
    Gfx* curr = workingMem;

    int scaleX;
    int scaleY;

    if (w == sw)
    {
        scaleX = 0x400;
    }
    else
    {
        scaleX = (sw << 10) / w;
    }

    if (h == sh)
    {
        scaleY = 0x400;
    }
    else
    {
        scaleY = (sh << 10) / h;
    }

    gSPTextureRectangle(
        curr++,
        x << 2, 
        y << 2,
        (x + w) << 2,
        (y + h) << 2,
        G_TX_RENDERTILE,
        sx << 5, sy << 5,
        scaleX,
        scaleY
    );

    writeDL(layer, workingMem, curr - workingMem);
}

void initSprites()
{
    for (int i = 0; i < MAX_LAYER_COUNT; ++i)
    {
        gLayerDL[i] = NULL;
        gCurrentLayerDL[i] = NULL;
        gLayerChunk[i] = NULL;
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
