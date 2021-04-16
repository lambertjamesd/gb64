
#ifndef _SPRITEFONT_H
#define _SPRITEFONT_H

#define ANSI_CHAR_COUNT 128

struct Character
{
    unsigned char x;
    unsigned char y;
    unsigned char w;
    unsigned char h;
};

struct CharacterDefinition
{
    char character;
    struct Character data;
};

struct Font
{
    struct Character characters[ANSI_CHAR_COUNT];
    short spriteLayer;
    short spaceWidth; 
};

void initFont(struct Font* font, int layer, int spaceWidth, struct CharacterDefinition* chars, int charCount);
void renderText(struct Font* font, const char* str, int x, int y);

void initGBFont();
extern struct Font gGBFont;

#endif