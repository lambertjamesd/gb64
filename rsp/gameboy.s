
#define GB_SCREEN_WD 160
#define GB_SCREEN_HT 144

#define GB_TILEMAP_W 32

#define GB_TILE_SIZE 16

#define GB_MAX_VISIBLE_TILES 22

#define GB_MAX_VISIBLE_SPRITES 10

#define TILE_ATTR_PALETTE       0x07
#define TILE_ATTR_VRAM_BANK     0x08
#define TILE_ATTR_H_FLIP        0x20
#define TILE_ATTR_V_FLIP        0x40
#define TILE_ATTR_PRIORITY      0x80

#define WINDOW_X_OFFSET         0x07

#define SPRITE_SIZE             0x4
#define SPRITE_MAX_COUNT        40
#define SPRITE_MAX_PER_LINE     10

#define SPRITE_Y                0x0
#define SPRITE_X                0x1
#define SPRITE_TILE             0x2
#define SPRITE_FLAGS            0x3

#define SPRITE_OFFSCREEN_X      168

#define SPRITE_SHIFT_Y          -16

#define SPRITE_FLAGS_PRIO       0x80
#define SPRITE_FLAGS_FLIP_Y     0x40
#define SPRITE_FLAGS_FLIP_X     0x20

#define MODE_3_FLAG 0x080
#define MODE_2_FLAG 0x100
#define MODE_3_FLAG_CLR 0x0200
#define MODE_2_FLAG_CLR 0x0800