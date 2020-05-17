
#include "z80_test.h"

int testRET_NZ(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = 0;
    z80->sp = 0x8020;
    expected = *z80;
    expected.pc = 1;

    memory[0] = Z80_RET_NZ;
    memory[1] = Z80_RET_NZ;

    memory[0x20] = 0x34;
    memory[0x21] = 0xE0;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("RET NZ", testOutput, z80, &expected) ||
        !testInt("RET NZ run result", testOutput, run, 2))
    {
        return 0;
    }
    
    z80->f = GB_FLAGS_Z;
    expected.f = GB_FLAGS_Z;
    expected.pc = 0x34E0;
    expected.sp = 0x8022;
    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("RET NZ branch", testOutput, z80, &expected) &&
        testInt("RET NZ branch run result", testOutput, run, 3) &&
        1
    ;
}

int testPOP_BC(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->sp = 0x8020;
    expected = *z80;
    expected.pc = 1;
    expected.sp = 0x8022;
    expected.b = 0x34;
    expected.c = 0xE0;

    memory[0] = Z80_POP_BC;

    memory[0x20] = 0x34;
    memory[0x21] = 0xE0;

    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("POP BC branch", testOutput, z80, &expected) &&
        testInt("POP BC branch run result", testOutput, run, 3) &&
        1
    ;
}

int testJP_NZ(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = 0;
    expected = *z80;
    expected.pc = 3;

    memory[0] = Z80_JP_NZ_a16;
    memory[1] = 0x00;
    memory[2] = 0x20;
    memory[3] = Z80_JP_NZ_a16;
    memory[4] = 0x00;
    memory[5] = 0x30;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("JP NZ", testOutput, z80, &expected) ||
        !testInt("JP NZ run result", testOutput, run, 3))
    {
        return 0;
    }
    
    z80->f = GB_FLAGS_Z;
    expected.f = GB_FLAGS_Z;
    expected.pc = 0x0030;
    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("JP NZ branch", testOutput, z80, &expected) &&
        testInt("JP NZ branch run result", testOutput, run, 3) &&
        1
    ;
}


int testJP(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = 0;
    expected = *z80;
    expected.pc = 0x0020;

    memory[0] = Z80_JP_a16;
    memory[1] = 0x00;
    memory[2] = 0x20;

    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("JP", testOutput, z80, &expected) &&
        testInt("JP run result", testOutput, run, 3) &&
        1
    ;
}

int testCALL_NZ(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = 0;
    z80->sp = 0x8022;
    expected = *z80;
    expected.pc = 3;

    memory[0] = Z80_CALL_NZ;
    memory[1] = 0x00;
    memory[2] = 0x20;
    memory[3] = Z80_CALL_NZ;
    memory[4] = 0x00;
    memory[5] = 0x30;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("CALL NZ", testOutput, z80, &expected) ||
        !testInt("CALL NZ run result", testOutput, run, 3))
    {
        return 0;
    }
    
    z80->f = GB_FLAGS_Z;
    expected.f = GB_FLAGS_Z;
    expected.pc = 0x0030;
    expected.sp = 0x8020;
    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("CALL NZ branch", testOutput, z80, &expected) &&
        testInt("CALL NZ branch run result", testOutput, run, 5) &&
        testInt("CALL NZ saved return", testOutput, memory[0x20], 0x0) &&
        testInt("CALL NZ saved return", testOutput, memory[0x21], 0x6) &&
        1
    ;
}

int testPUSH_BC(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = 0;
    z80->sp = 0x8022;
    z80->b = 0x45;
    z80->c = 0x24;
    expected = *z80;
    expected.pc = 1;
    expected.sp = 0x8020;

    memory[0] = Z80_PUSH_BC;

    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("PUSH BC", testOutput, z80, &expected) &&
        testInt("PUSH BC run result", testOutput, run, 4) &&
        testInt("PUSH BC pushed value", testOutput, memory[0x20], 0x45) &&
        testInt("PUSH BC pushed value", testOutput, memory[0x21], 0x24) &&
        1
    ;
}

int run0xCTests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    return 
        testRET_NZ(z80, memoryMap, memory, testOutput) &&
        testPOP_BC(z80, memoryMap, memory, testOutput) &&
        testJP_NZ(z80, memoryMap, memory, testOutput) &&
        testJP(z80, memoryMap, memory, testOutput) &&
        testCALL_NZ(z80, memoryMap, memory, testOutput) &&
        testPUSH_BC(z80, memoryMap, memory, testOutput) &&
        testSingleADD(z80, memoryMap, memory, testOutput, d8_REGISTER_INDEX, Z80_ADD_A_d8, 0) &&
        testSingleADD(z80, memoryMap, memory, testOutput, d8_REGISTER_INDEX, Z80_ADD_A_d8, 1) &&
        1
    ;
}