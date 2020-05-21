
#ifndef _GAMEBOY_H
#define _GAMEBOY_H

#include "rom.h"
#include "z80.h"
#include "memory_map.h"

struct GameBoy
{
    struct ROMLayout* rom;
    struct Memory* memory;
    struct Z80State cpu;
};

#endif