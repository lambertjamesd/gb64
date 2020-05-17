#include "z80.h"

struct Z80State gZ80;
struct Memory gMemory;

void zeroMemory(void* memory, int size)
{
    int* asInt;
    unsigned char* asChar;

    asInt = (int*)memory;

    while (size > 3) {
        *asInt = 0;
        ++asInt;
        size -= 4;
    }

    asChar = (unsigned char*)asInt;
    
    while (size > 0) {
        *asChar = 0;
        ++asChar;
        --size;
    }
}

void initializeZ80(struct Z80State* state)
{
    state->a = 0; state->f = 0; state->b = 0; state->c = 0;
    state->d = 0; state->e = 0; state->h = 0; state->l = 0;
    state->sp = 0; state->pc = 0;
    state->stopReason = STOP_REASON_NONE;
    state->interrupts = 0;
}