
#define PPUTask_output         0
#define PPUTask_memorySource   4
#define PPUTask_graphicsSource 8
#define PPUTask_flags          12
#define PPUTask_lcdc           14

#define REG_LCDC_LCD_ENABLE    0x80
#define REG_LCDC_GDB_TILE_MAP  0x08

#define GraphicsMemory_tilemap0          0x1800
#define GraphicsMemory_tilemapOffset     0x2000

#define PPUTask_ly             15
#define PPUTask_scy            16
#define PPUTask_scx            17
#define PPUTask_wy             18
#define PPUTask_wx             19
#define PPUTask_padding        20

#define PPUTask_sizeof         24