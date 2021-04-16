
#ifndef _SPRITE_H
#define _SPRITE_H

#include <ultra64.h>

#define MAX_LAYER_COUNT     8

#define SPRITE_FONT_LAYER       0

void setLayerGraphics(int layer, Gfx* graphics);
void drawSprite(int layer, int x, int y, int w, int h, int sx, int sy, int sw, int sh);

void initSprites();
void finishSprites(Gfx** out);

#endif