
#include "z80_test.h"

int testLDH_a8_A(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = GB_FLAGS_C;
    z80->sp = 0x8020;
    expected = *z80;
    expected.pc = 1;

    memory[0] = Z80_LDH_a8_A;
    memory[1] = 0x40;

    memory[0x20] = 0xE0;
    memory[0x21] = 0x34;

    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("RET NC branch", testOutput, z80, &expected) &&
        testInt("RET NC branch run result", testOutput, run, 3) &&
        1
    ;
}

int run0xETests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    return 
        testLDH_a8_A(z80, memoryMap, memory, testOutput) &&
        1
    ;
}