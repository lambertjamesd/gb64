
#ifndef _GAMEBOY_H
#define _GAMEBOY_H

#include "rom.h"
#include "cpu.h"
#include "memory_map.h"

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

#endif