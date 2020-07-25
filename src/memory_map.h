
#ifndef _MEMORY_MAP_H
#define _MEMORY_MAP_H

#include <ultra64.h>
#include "rom.h"
#include "audio.h"
#include "bool.h"

#define MEMORY_MAP_SIZE 16
#define MEMORY_MAP_SEGMENT_SIZE 0x1000
#define MAX_RAM_SIZE 0x8000
#define RAM_BANK_SIZE 0x2000
#define MISC_MEMORY_SIZE 512

#define SPRITE_COUNT      40

#define SPRITE_F_PRIORITY 0x80
#define SPRITE_F_FLIPX    0x40
#define SPRITE_F_FLIPY    0x20
#define SPRITE_F_PALETTE  0x10

#define MISC_START              0xFE00
#define REGISTERS_START         0xFF00

#define REG_JOYP        0xFF00
#define REG_SERIAL_DATA 0xFF01
#define REG_SERIAL      0xFF02
#define _REG_JOYSTATE   0xFF03
#define REG_DIV         0xFF04 
#define REG_TIMA        0xFF05 
#define REG_TMA         0xFF06
#define REG_TAC         0xFF07

#define REG_NR10        0xFF10
#define REG_NR11        0xFF11
#define REG_NR12        0xFF12
#define REG_NR13        0xFF13
#define REG_NR14        0xFF14

#define REG_NR21        0xFF16
#define REG_NR22        0xFF17
#define REG_NR23        0xFF18
#define REG_NR24        0xFF19

#define REG_NR30        0xFF1A
#define RER_NR30_ENABLED    0x80

#define REG_NR31        0xFF1B
#define REG_NR32        0xFF1C
#define REG_NR33        0xFF1D
#define REG_NR34        0xFF1E

#define REG_NR41        0xFF20
#define REG_NR42        0xFF21
#define REG_NR43        0xFF22
#define REG_NR44        0xFF23

#define REG_NR50        0xFF24
#define REG_NR51        0xFF25
#define REG_NR52        0xFF26
#define REG_NR52_ENABLED 0x80

#define REG_WAVE_PAT    0xFF30

#define REG_LCDC        0xFF40
#define REG_LCD_STAT    0xFF41
#define REG_LCD_STAT_MODE 0x3
#define REG_SCY         0xFF42
#define REG_SCX         0xFF43
#define REG_LY          0xFF44
#define REG_LCY         0xFF45
#define REG_BGP         0xFF47
#define REG_OBP0        0xFF48
#define REG_OBP1        0xFF49
#define REG_WY          0xFF4A
#define REG_WX          0xFF4B
#define REG_KEY1        0xFF4D
#define REG_VBK         0xFF4F
#define REG_VBK_MASK    0x1

#define REG_UNLOAD_BIOS 0xFF50
#define REG_HDMA5       0xFF55

#define REG_SVBK        0xFF70
#define REG_SVBK_MASK   0x7

#define REG_RTC_S       0xFF78
#define REG_RTC_M       0xFF79
#define REG_RTC_H       0xFF7A
#define REG_RTC_DL      0xFF7B
#define REG_RTC_DH      0xFF7C

#define REG_RTC_DH_HIGH 0x1
#define REG_RTC_DH_HALT 0x40
#define REG_RTC_DH_C    0x80

#define REG_KEY1_PREPARE_SWITCH  0x1
#define REG_KEY1_CURRENT_SPEED  0x80

#define REG_INT_REQUESTED   0xFF0F
#define REG_INT_ENABLED     0xFFFF

#define TILEMAP_W   32
#define TILEMAP_H   32

#define MEMORY_VRAM_BANK_INDEX  0x8
#define MEMORY_RAM_BANK_INDEX   0xD

#define WRITE_REGISTER_DIRECT(mm, addr, val) (mm)->miscBytes[(addr) - MISC_START] = (val)
#define READ_REGISTER_DIRECT(mm, addr) ((mm)->miscBytes[(addr) - MISC_START])
#define GET_REGISTER_ADDRESS(mm, addr) ((mm)->miscBytes + (addr) - MISC_START)

/*
    Sprite.flags
  Bit7   OBJ-to-BG Priority (0=OBJ Above BG, 1=OBJ Behind BG color 1-3)
         (Used for both BG and Window. BG color 0 is always behind OBJ)
  Bit6   Y flip          (0=Normal, 1=Vertically mirrored)
  Bit5   X flip          (0=Normal, 1=Horizontally mirrored)
  Bit4   Palette number  **Non CGB Mode Only** (0=OBP0, 1=OBP1)
  Bit3   Tile VRAM-Bank  **CGB Mode Only**     (0=Bank 0, 1=Bank 1)
  Bit2-0 Palette number  **CGB Mode Only**     (OBP0-7)
 */

#define SPRITE_FLAGS_PRIORITY       0x80
#define SPRITE_FLAGS_Y_FLIP         0x40
#define SPRITE_FLAGS_X_FLIP         0x20
#define SPRITE_FLAGS_DMA_PALETTE    0x10
#define SPRITE_FLAGS_VRAM_BANK      0x08
#define SPRITE_FLAGS_GBC_PALETTE    0x07

#define DEBUG_INSTRUCTION  0xD3

enum BreakpointType {
    BreakpointTypeNone,
    BreakpointTypeUser,
    BreakpointTypeStep,
};

struct Breakpoint {
    u16 address;
    u8 existingInstruction;
    u8 breakpointType;
    u8* memoryAddress;
};

#define USER_BREAK_POINTS       8
#define SYSTEM_BREAK_POINTS     2
#define BREAK_POINT_COUNT       (USER_BREAK_POINTS + SYSTEM_BREAK_POINTS)

#define SYSTEM_BREAK_POINT_START    USER_BREAK_POINTS

struct Sprite {
    unsigned char y;
    unsigned char x;
    unsigned char tile;
    unsigned char flags;
};

struct MiscMemory {
    struct Sprite sprites[SPRITE_COUNT];
    int romBankLower;
    int romBankUpper;
    int ramRomSelect;
    int biosLoaded;
    u64 time;
    unsigned char unused[0x48];
    unsigned char controlRegisters[0x80];
    unsigned char fastRam[128]; // last byte is actually interrupt register
};

struct Tile {
    unsigned short rows[8];
};

#define PALETTE_COUNT   64
#define OBJ_PALETTE_INDEX_START 32

struct GraphicsMemory {
    struct Tile tiles[384];
    unsigned char tilemap0[1024];
    unsigned char tilemap1[1024];

    struct Tile gbcTiles[384];
    unsigned char tilemap0Atts[1024];
    unsigned char tilemap1Atts[1024];
    
    // first 32 colors are background pallettes
    // last 32 colors are obj palettes
    u16 colorPalettes[PALETTE_COUNT];
};

struct Memory;

typedef void (*RegisterWriter)(struct Memory*, int addr, int value);

typedef void (*ASMHook)();

struct MBCData {
    RegisterWriter bankSwitch;
    u16 id;
    u16 flags;
};

struct Memory {
    void* memoryMap[MEMORY_MAP_SIZE];
    ASMHook cartRamWrite;
    ASMHook cartRamRead;
    RegisterWriter bankSwitch;
    union {
        struct MiscMemory misc;
        unsigned char miscBytes[sizeof(struct MiscMemory)];
    };
    unsigned char internalRam[MAX_RAM_SIZE];
    unsigned char* cartRam;
    union {
        struct GraphicsMemory vram;
        unsigned char vramBytes[sizeof(struct GraphicsMemory)];
    };
    struct ROMLayout* rom;
    struct AudioRenderState audio;
    struct Breakpoint breakpoints[BREAK_POINT_COUNT];
    u8* timerMemoryBank;
    struct MBCData* mbc;
};

#define MBC_FLAGS_RAM       0x1
#define MBC_FLAGS_BATTERY   0x2
#define MBC_FLAGS_TIMER     0x4
#define MBC_FLAGS_RUMBLE    0x8


void initMemory(struct Memory* memory, struct ROMLayout* rom);

void handleMBC3Write(struct Memory* memory, int addr, int value);
void writeMBC3ClockRegisters(u64 time, u8* target);
u64 readMBC3ClockRegisters(u8* source);

#endif