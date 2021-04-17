
#ifndef _SPRITEFONT_H
#define _SPRITEFONT_H

#define ANSI_CHAR_COUNT 128

#include "sprite.h"

struct CharacterDefinition
{
    char character;
    struct SpriteTile data;
};

struct Font
{
    struct SpriteTile characters[ANSI_CHAR_COUNT];
    short spriteLayer;
    short spaceWidth; 
};

void initFont(struct Font* font, int layer, int spaceWidth, struct CharacterDefinition* chars, int charCount);
void renderText(struct Font* font, const char* str, int x, int y, int scaleShift);

void initGBFont();
extern struct Font gGBFont;

#endif