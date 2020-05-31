
#ifndef _GAMEBOY_H
#define _GAMEBOY_H

#include <ultra64.h>
#include "rom.h"
#include "cpu.h"
#include "memory_map.h"

#define GB_BUTTON_A         0x01
#define GB_BUTTON_B         0x02
#define GB_BUTTON_SELECT    0x04
#define GB_BUTTON_START     0x08

#define GB_BUTTON_RIGHT     0x10
#define GB_BUTTON_LEFT      0x20
#define GB_BUTTON_UP        0x40
#define GB_BUTTON_DOWN      0x80

struct GameBoy
{
    struct Memory memory;
    struct CPUState cpu;
};

extern struct GameBoy gGameboy;

void initGameboy(struct GameBoy* gameboy, struct ROMLayout* rom);

void requestInterrupt(struct GameBoy* gameboy, int interrupt);

/**
 * if targetMemory is null then the drawing routines are skipped
 */
void emulateFrame(struct GameBoy* gameboy, void* targetMemory);

void handleInput(struct GameBoy* gameboy, OSContPad* pad);

#endif