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

    return 1;
}

int testSpriteSorting(char* testOutput)
{
    int testCycle = 0;
    struct Sprite unsorted[SPRITE_COUNT];
    struct Sprite sorted[SPRITE_COUNT];
    int spriteIndex;

    for (testCycle = 0; testCycle < 400; ++testCycle)
    {
        for (spriteIndex = 0; spriteIndex < SPRITE_COUNT; ++spriteIndex)
        {
            unsorted[spriteIndex].x = (unsigned int)(spriteIndex * 4627373 + testCycle * 620603677) % 170;
            unsorted[spriteIndex].y = (unsigned int)(spriteIndex * 54144919 + testCycle * 4615003) % 170;
        }

        int preparedCount = 0;
        prepareSprites(unsorted, sorted, &preparedCount, 1);

        if (!assertSpritesOrdered(testOutput, sorted, preparedCount))
        {
            return 0;
        }
    }

    return 1;
}

int testGraphics(char* testOutput)
{
    return 
        testSpriteSorting(testOutput) &&
    1;
}