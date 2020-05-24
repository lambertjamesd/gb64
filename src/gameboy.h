
#ifndef _GAMEBOY_H
#define _GAMEBOY_H

#include "rom.h"
#include "z80.h"
#include "memory_map.h"

struct GameBoy
{
    struct Memory memory;
    struct Z80State cpu;
};

extern struct GameBoy gGameboy;

void initGameboy(struct GameBoy* gameboy, struct ROMLayout* rom);

void requestInterrupt(struct GameBoy* gameboy, int interrupt);

#endif