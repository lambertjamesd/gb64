#include "./graphics_test.h"
#include "../graphics.h"

int assertSpritesOrdered(char* testOutput, struct Sprite* sprites, int spriteCount)
{
    int i;
    for (i = 0; i < spriteCount - 1; ++i)
    {
        if (compareSprites(sprites[i], sprites[i + 1]) > 0)
        {
            sprintf(testOutput, "Sprites not ordered");
            return 0;
        }
    }
}

int testGraphics(char* testOutput)
{
    return 1;
}