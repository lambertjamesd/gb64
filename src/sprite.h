
#ifndef _SPRITE_H
#define _SPRITE_H

#include <ultra64.h>

#define MAX_LAYER_COUNT     8

#define SPRITE_BORDER_LAYER     0
#define SPRITE_CBUTTONS_LAYER   1
#define SPRITE_TRIGGERS_LAYER   2
#define SPRITE_DPAD_LAYER       3
#define SPRITE_FACE_LAYER       4
#define SPRITE_FONT_LAYER       5

struct SpriteTile
{
    char x;
    char y;
    char w;
    char h;
};

void setLayerGraphics(int layer, Gfx* graphics);
void spriteDraw(int layer, int x, int y, int w, int h, int sx, int sy, int scaleShiftX, int scaleShiftY);
void spriteDrawTile(int layer, int x, int y, int w, int h, struct SpriteTile tile);
void spriteSetColor(int layer, char r, char g, char b, char a);

void initSprites();
void finishSprites(Gfx** out);

#endif