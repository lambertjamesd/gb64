#include "z80.h"

void runTests(char* testOutput) {
    struct Z80State z80;
    unsigned char memory[512];
    zeroMemory(memory, 512);
    initializeZ80(&z80);

    z80.a = 1;
    z80.f = 2;
    z80.b = 5;
    z80.c = 13;
    z80.d = 12;
    z80.e = 18;
    z80.h = 23;
    z80.l = 19;
    z80.sp = 256;
    z80.pc = 128;

	sprintf(
        testOutput,
        "Test ASM: %d", 
        runZ80CPU(&z80, memory, 10)
    );
}