
#include "z80_test.h"

int testLDH_a8_A(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->a = 0x3E;
    expected = *z80;
    expected.pc = 2;

    memory->internalRam[0] = Z80_LDH_a8_A;
    memory->internalRam[1] = 0xF0;

    run = runZ80CPU(z80, memory, 1);

    return testZ80State("RET LDH a8 A branch", testOutput, z80, &expected) &&
        testInt("RET LDH a8 A branch run result", testOutput, run, 3) &&
        testInt("RET LDH a8 A store result", testOutput, memory->miscBytes[0x1F0], 0x3E) &&
        1
    ;
}

int run0xETests(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    return 
        testLDH_a8_A(z80, memory, testOutput) &&
        1
    ;
}