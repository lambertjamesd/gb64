
#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "memory_map.h"
#include "gameboy.h"

#define GB_SCREEN_TILE_H    20

#define MAX_PALLETE_SIZE (PALETTE_COUNT * GB_SCREEN_H)

#define GB_SCREEN_W         160
#define GB_SCREEN_H         144
#define GB_SCREEN_LINES     154

#define GB_RENDER_STRIP_HEIGHT  12

#define RENDER_TO_X 80
#define RENDER_TO_Y 48

#define SPRITE_WIDTH        8
#define SPRITE_BASE_HEIGHT  8
#define SPRITE_Y_OFFSET     16

#define TILE_ATTR_PALETTE       0x07
#define TILE_ATTR_VRAM_BANK     0x08
#define TILE_ATTR_H_FLIP        0x20
#define TILE_ATTR_V_FLIP        0x40
#define TILE_ATTR_PRIORITY      0x80

/*
  Bit 0 - BG Display (for CGB see below) (0=Off, 1=On)
  Bit 1 - OBJ (Sprite) Display Enable    (0=Off, 1=On)
  Bit 2 - OBJ (Sprite) Size              (0=8x8, 1=8x16)
  Bit 3 - BG Tile Map Display Select     (0=9800-9BFF, 1=9C00-9FFF)
  Bit 4 - BG & Window Tile Data Select   (0=8800-97FF, 1=8000-8FFF)
  Bit 5 - Window Display Enable          (0=Off, 1=On)
  Bit 6 - Window Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
  Bit 7 - LCD Display Enable             (0=Off, 1=On)
  */
#define LCDC_BG_ENABLE          0x01
#define LCDC_OBJ_ENABLE         0x02
#define LCDC_OBJ_SIZE           0x04
#define LCDC_BG_TILE_MAP        0x08
#define LCDC_BG_TILE_DATA       0x10
#define LCDC_WIN_E              0x20
#define LCDC_WIN_TILE_MAP       0x40
#define LCDC_LCD_E              0x80

#define WINDOW_X_OFFSET         0x7

#define MODE_2_CYCLES           20
#define MODE_3_CYCLES           43
#define CYCLES_TIL_LINE_RENDER  (MODE_2_CYCLES + 1)
#define CYCLES_PER_LINE         114
#define V_BLANK_LINES           10
#define CYCLES_PER_FRAME        (CYCLES_PER_LINE * (GB_SCREEN_H + V_BLANK_LINES))

#define READ_PIXEL_INDEX(pixel, x) ((x == 7) ? ((((pixel) >> 8) & 0x1) | (((pixel) << 1) & 0x2)) : ((((pixel) >> (15 - x)) & 0x1) | (((pixel) >> (6 - x)) & 0x2)))

// GGGRRRRR 1BBBBBGG to RRRRRGGGGGBBBBB1
#define GBC_TO_N64_COLOR(color) (((color) << 3 & 0xF100) | ((color) >> 7 & 0x01C0 >> 7) | ((color) << 9 & 0x0600) | ((color) >> 1 & 0x003E) | 0x1)

extern u16 gScreenPalette[MAX_PALLETE_SIZE];

struct RenderBlockInformation {
  u16 row;
  u16 palleteWriteIndex;
};

struct GraphicsState {
    struct Sprite sortedSprites[SPRITE_COUNT];
    u8 spriteIndexBuffer[GB_SCREEN_W];
    struct GameboyGraphicsSettings settings;
    int spriteCount;
    u16 gbc;
    u16 row;
    u16 lastRenderedRow;
    u16 palleteWriteIndex;
};

void applyGrayscalePallete(struct GraphicsState* state);
void beginScreenDisplayList(struct GameboyGraphicsSettings* settings, void* colorBuffer);
void prepareGraphicsPallete(struct GraphicsState* state);
void renderScreenBlock(struct GraphicsState* state);

void initGraphicsState(
    struct Memory* memory,
    struct GraphicsState* state,
    struct GameboyGraphicsSettings* settings,
    int gbc,
    void* colorBuffer
);

void finishScreen(struct GraphicsState* state);
void rerenderLastFrame(struct GameboyGraphicsSettings* settings, void* colorBuffer);
void waitForRDP();
int palleteUsedCount();

#endif