#include "z80.h"

struct Z80State gZ80;
// 16K memory map
unsigned char gbMemory[GB_MEMORY_MAP_SIZE];

extern void zeroMemory(unsigned char* memory, int size)
{
    int* asInt;

    asInt = (int*)memory;

    while (size > 3) {
        *asInt = 0;
        ++asInt;
        size -= 4;
    }

    memory = (unsigned char*)asInt;
    
    while (size > 0) {
        *memory = 0;
        ++memory;
        --size;
    }
}

extern void initializeZ80(struct Z80State* state)
{
    state->afbc = 0;
    state->dehl = 0;
    state->sp_pc = 0;
    state->state = 0;
}