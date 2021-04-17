
#include "spritefont.h"
#include "sprite.h"

void initFont(struct Font* font, int layer, int spaceWidth, struct CharacterDefinition* chars, int charCount)
{
    font->spriteLayer = layer;
    font->spaceWidth = spaceWidth;

    for (int i = 0; i < ANSI_CHAR_COUNT; ++i)
    {
        font->characters[i].w = 0;
    }

    for (int i = 0; i < charCount; ++i)
    {
        font->characters[chars[i].character] = chars[i].data;
    }
}

void setFontColor(struct Font* font, char r, char g, char b)
{

}

void renderText(struct Font* font, const char* str, int x, int y, int scaleShift)
{
    while (*str)
    {
        struct SpriteTile curr = font->characters[*str];
        if (curr.w)
        {
            spriteDraw(font->spriteLayer, x, y, curr.w << scaleShift, curr.h << scaleShift, curr.x, curr.y, scaleShift, scaleShift);
            x += curr.w << scaleShift;
        }
        else if (*str == ' ')
        {
            x += font->spaceWidth << scaleShift;
        }

        ++str;
    }
}

struct CharacterDefinition gGBFontDef[] = {
    {'A', 0*8, 0*8, 8, 8},
    {'B', 1*8, 0*8, 8, 8},
    {'C', 2*8, 0*8, 8, 8},
    {'D', 3*8, 0*8, 8, 8},
    {'E', 4*8, 0*8, 8, 8},
    {'F', 5*8, 0*8, 8, 8},
    {'G', 6*8, 0*8, 8, 8},
    {'H', 7*8, 0*8, 8, 8},
    {'I', 0*8, 1*8, 8, 8},
    {'J', 1*8, 1*8, 8, 8},
    {'K', 2*8, 1*8, 8, 8},
    {'L', 3*8, 1*8, 8, 8},
    {'M', 4*8, 1*8, 8, 8},
    {'N', 5*8, 1*8, 8, 8},
    {'O', 6*8, 1*8, 8, 8},
    {'P', 7*8, 1*8, 8, 8},
    {'Q', 0*8, 2*8, 8, 8},
    {'R', 1*8, 2*8, 8, 8},
    {'S', 2*8, 2*8, 8, 8},
    {'T', 3*8, 2*8, 8, 8},
    {'U', 4*8, 2*8, 8, 8},
    {'V', 5*8, 2*8, 8, 8},
    {'W', 6*8, 2*8, 8, 8},
    {'X', 7*8, 2*8, 8, 8},
    {'Y', 0*8, 3*8, 8, 8},
    {'Z', 1*8, 3*8, 8, 8},
    {'a', 0*8, 0*8, 8, 8},
    {'b', 1*8, 0*8, 8, 8},
    {'c', 2*8, 0*8, 8, 8},
    {'d', 3*8, 0*8, 8, 8},
    {'e', 4*8, 0*8, 8, 8},
    {'f', 5*8, 0*8, 8, 8},
    {'g', 6*8, 0*8, 8, 8},
    {'h', 7*8, 0*8, 8, 8},
    {'i', 0*8, 1*8, 8, 8},
    {'j', 1*8, 1*8, 8, 8},
    {'k', 2*8, 1*8, 8, 8},
    {'l', 3*8, 1*8, 8, 8},
    {'m', 4*8, 1*8, 8, 8},
    {'n', 5*8, 1*8, 8, 8},
    {'o', 6*8, 1*8, 8, 8},
    {'p', 7*8, 1*8, 8, 8},
    {'q', 0*8, 2*8, 8, 8},
    {'r', 1*8, 2*8, 8, 8},
    {'s', 2*8, 2*8, 8, 8},
    {'t', 3*8, 2*8, 8, 8},
    {'u', 4*8, 2*8, 8, 8},
    {'v', 5*8, 2*8, 8, 8},
    {'w', 6*8, 2*8, 8, 8},
    {'x', 7*8, 2*8, 8, 8},
    {'y', 0*8, 3*8, 8, 8},
    {'z', 1*8, 3*8, 8, 8},
    {'0', 2*8, 3*8, 8, 8},
    {'1', 3*8, 3*8, 8, 8},
    {'2', 4*8, 3*8, 8, 8},
    {'3', 5*8, 3*8, 8, 8},
    {'4', 6*8, 3*8, 8, 8},
    {'5', 7*8, 3*8, 8, 8},
    {'6', 0*8, 4*8, 8, 8},
    {'7', 1*8, 4*8, 8, 8},
    {'8', 2*8, 4*8, 8, 8},
    {'9', 3*8, 4*8, 8, 8},
    {'.', 4*8, 4*8, 8, 8},
    {'?', 5*8, 4*8, 8, 8},
    {'!', 6*8, 4*8, 8, 8},
    {',', 7*8, 4*8, 8, 8},
    {'@', 0*8, 5*8, 8, 8},
    {'[', 1*8, 5*8, 8, 8},
    {']', 2*8, 5*8, 8, 8},
    {'\'', 3*8, 5*8, 8, 8},
    {'\"', 4*8, 5*8, 8, 8},
    {'\\', 5*8, 5*8, 8, 8},
    {'+', 6*8, 5*8, 8, 8},
    {'-', 7*8, 5*8, 8, 8},
    {'=', 0*8, 6*8, 8, 8},
    {'/', 1*8, 6*8, 8, 8},
    {'(', 2*8, 6*8, 8, 8},
    {')', 3*8, 6*8, 8, 8},
};

struct Font gGBFont;

void initGBFont()
{
    initFont(&gGBFont, SPRITE_FONT_LAYER, 8, gGBFontDef, sizeof(gGBFontDef) / sizeof(*gGBFontDef));
}