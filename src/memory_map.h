
#ifndef _MEMORY_MAP_H
#define _MEMORY_MAP_H

#define MEMORY_MAP_SIZE 16
#define RAM_BANK_SIZE 8192
#define MAX_RAM_BANKS 4
#define VRAM_SIZE 8192
#define MAX_VRAM_BANKS 2
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

#define MISC_START              0xFE00
#define REGISTERS_START         0xFF00
#define REGISTER_WRITER_COUNT   0x08

struct Sprite {
    unsigned char x;
    unsigned char y;
    unsigned char pattern;
    unsigned char flags;
};

struct MiscMemory {
    struct Sprite sprites[SPRITE_COUNT];
    unsigned char unused[0x60];
    unsigned char controlRegisters[0x80];
    unsigned char fastRam[128]; // last byte is actually interrupt register
};

struct Memory;

typedef void (*RegisterWriter)(struct Memory*, int addr, unsigned char value);

struct Memory {
    void* memoryMap[MEMORY_MAP_SIZE];
    RegisterWriter registerWriters[REGISTER_WRITER_COUNT];
    RegisterWriter bankSwitch;
    union {
        struct MiscMemory misc;
        unsigned char miscBytes[512];
    };
    unsigned char internalRam[RAM_BANK_SIZE];
    unsigned char* pagedRam;
    unsigned char vram[VRAM_SIZE * MAX_VRAM_BANKS];
};

#endif