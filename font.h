/*
	Fonts: Definitions and externals.
*/

#include <PR/sp.h>

typedef struct sp2d_font_struct {
    char *index_string;
    Bitmap *bitmaps;
    char *img;
} Font;


extern Font lcase_font;

void text_sprite( Sprite *, char *, Font *, int , int );

