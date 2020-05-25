
#ifndef _MEMORY_MAP_H
#define _MEMORY_MAP_H

#include <ultra64.h>
#include "rom.h"

#define MEMORY_MAP_SIZE 16
#define MEMORY_MAP_SEGMENT_SIZE 0x1000
#define MAX_RAM_SIZE 0x8000
#define RAM_BANK_SIZE 0x2000
#define MISC_MEMORY_SIZE 512

#define SPRITE_COUNT      40

#define SPRITE_F_PRIORITY 0x80
#define SPRITE_F_FLIPX    0x40
#define SPRITE_F_FLIPY    0x20
#define SPRITE_F_PALLETE  0x10

#define MISC_START              0xFE00
#define REGISTERS_START         0xFF00
#define REGISTER_WRITER_COUNT   0x08

#define REG_JOYP        0xFF00
#define _REG_JOYSTATE   0xFF03
#define REG_DIV         0xFF04 
#define REG_TIMA        0xFF05 
#define REG_TMA         0xFF06
#define REG_TAC         0xFF07

#define REG_LCDC        0xFF40
#define REG_SCY         0xFF42
#define REG_SCX         0xFF43

#define REG_INT_REQUESTED   0xFF0F
#define REG_INT_ENABLED     0xFFFF

#define TILEMAP_W   32
#define TILEMAP_H   32

#define MEMORY_VRAM_BANK_INDEX  0x8
#define MEMORY_RAM_BANK_INDEX   0xD

#define WRITE_REGISTER_DIRECT(mm, addr, val) (mm)->miscBytes[(addr) - MISC_START] = (val)
#define READ_REGISTER_DIRECT(mm, addr) ((mm)->miscBytes[(addr) - MISC_START])

struct Sprite {
    unsigned char x;
    unsigned char y;
    unsigned char pattern;
    unsigned char flags;
};

struct MiscMemory {
    struct Sprite sprites[SPRITE_COUNT];
    u16 colorPalletes[64];
    unsigned char unused[0x58];
    unsigned char controlRegisters[0x80];
    unsigned char fastRam[128]; // last byte is actually interrupt register
};

struct Tile {
    unsigned short rows[8];
};

struct GraphicsMemory {
    struct Tile tiles[384];
    unsigned char tilemap0[1024];
    unsigned char tilemap1[1024];

    struct Tile gbcTiles[384];
    unsigned char tilemap0Atts[1024];
    unsigned char tilemap1Atts[1024];
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
    unsigned char internalRam[MAX_RAM_SIZE];
    unsigned char* cartRam;
    union {
        struct GraphicsMemory vram;
        unsigned char vramBytes[sizeof(struct GraphicsMemory)];
    };
    struct ROMLayout* rom;
};

void initMemory(struct Memory* memory, struct ROMLayout* rom);

#endif