
#ifndef _TEXTURES_H
#define _TEXTURES_H

#include <ultra64.h>

extern char __attribute__((aligned(8))) gbfont_img[];
extern unsigned char __attribute__((aligned(8))) tex_cbuttons[];
extern unsigned char __attribute__((aligned(8))) tex_dpad[];
extern unsigned char __attribute__((aligned(8))) tex_facebuttons[];
extern unsigned char __attribute__((aligned(8))) tex_guiitems[];
extern unsigned char __attribute__((aligned(8))) tex_triggers[];

extern Gfx gUseFontTexture[];

#endif