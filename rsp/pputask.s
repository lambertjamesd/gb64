
#define PPUTask_output         0
#define PPUTask_memorySource   4
#define PPUTask_graphicsSource 8
#define PPUTask_flags          12
#define PPUTask_lcdc           14

#define LCDC_BG_ENABLE          0x01
#define LCDC_OBJ_ENABLE         0x02
#define LCDC_OBJ_SIZE           0x04
#define LCDC_BG_TILE_MAP        0x08
#define LCDC_BG_TILE_DATA       0x10
#define LCDC_WIN_E              0x20
#define LCDC_WIN_TILE_MAP       0x40
#define LCDC_LCD_E              0x80


#define Memory_misc_sprites     0x50

#define GraphicsMemory_tilemap0          0x1800
#define GraphicsMemory_tilemapOffset     0x2000

#define PPUTask_ly             15
#define PPUTask_scy            16
#define PPUTask_scx            17
#define PPUTask_wy             18
#define PPUTask_wx             19
#define PPUTask_performance    20

#define PPUTask_sizeof         24