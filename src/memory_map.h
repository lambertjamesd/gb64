

#define MEMORY_MAP_SIZE 128
#define RAM_BANK_SIZE 8192
#define MAX_RAM_BANKS 4
#define VRAM_SIZE 8192
#define MISC_MEMORY_SIZE 512

#define MM_ROM_0        0
#define MM_ROM_SWITCH   4
#define MM_VRAM         8
#define MM_RAM_SWITCH   10
#define MM_RAM          12
#define MM_RAM_ECHO     14
#define MM_MISC         15

#define SPRITE_COUNT      40

#define SPRITE_F_PRIORITY 0x80
#define SPRITE_F_FLIPX    0x40
#define SPRITE_F_FLIPY    0x20
#define SPRITE_F_PALLETE  0x10

struct Sprite {
    unsigned char x;
    unsigned char y;
    unsigned char pattern;
    unsigned char flags;
};

struct MiscMemory {
    struct Sprite sprites[SPRITE_COUNT];
    unsigned char ioPorts[224];
    unsigned char fastRam[128]; // last byte is actually interrupt register
};

struct Memory {
    void* memoryMap[MEMORY_MAP_SIZE];
    unsigned char internalRam[RAM_BANK_SIZE];
    unsigned char pagedRam[RAM_BANK_SIZE * MAX_RAM_BANKS];
    unsigned char vram[VRAM_SIZE];
    struct MiscMemory misc; 
};