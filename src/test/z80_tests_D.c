
#include "z80_test.h"

int testRET_NC(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = GB_FLAGS_C;
    z80->sp = 0x8020;
    expected = *z80;
    expected.pc = 1;

    memory[0] = Z80_RET_NC;
    memory[1] = Z80_RET_NC;

    memory[0x20] = 0xE0;
    memory[0x21] = 0x34;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("RET NC", testOutput, z80, &expected) ||
        !testInt("RET NC run result", testOutput, run, 2))
    {
        return 0;
    }
    
    z80->f = 0;
    expected.f = 0;
    expected.pc = 0x34E0;
    expected.sp = 0x8022;
    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("RET NC branch", testOutput, z80, &expected) &&
        testInt("RET NC branch run result", testOutput, run, 3) &&
        1
    ;
}

int testPOP_DE(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->sp = 0x8020;
    expected = *z80;
    expected.pc = 1;
    expected.sp = 0x8022;
    expected.d = 0x34;
    expected.e = 0xE0;

    memory[0] = Z80_POP_DE;

    memory[0x20] = 0xE0;
    memory[0x21] = 0x34;

    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("POP DE branch", testOutput, z80, &expected) &&
        testInt("POP DE branch run result", testOutput, run, 3) &&
        1
    ;
}

int testJP_NC(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = GB_FLAGS_C;
    expected = *z80;
    expected.pc = 3;

    memory[0] = Z80_JP_NC_a16;
    memory[1] = 0x20;
    memory[2] = 0x00;
    memory[3] = Z80_JP_NC_a16;
    memory[4] = 0x30;
    memory[5] = 0x00;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("JP NC", testOutput, z80, &expected) ||
        !testInt("JP NC run result", testOutput, run, 3))
    {
        return 0;
    }
    
    z80->f = 0;
    expected.f = 0;
    expected.pc = 0x0030;
    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("JP NC branch", testOutput, z80, &expected) &&
        testInt("JP NC branch run result", testOutput, run, 3) &&
        1
    ;
}

int testCALL_NC(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = GB_FLAGS_C;
    z80->sp = 0x8022;
    expected = *z80;
    expected.pc = 3;

    memory[0] = Z80_CALL_NC;
    memory[1] = 0x20;
    memory[2] = 0x00;
    memory[3] = Z80_CALL_NC;
    memory[4] = 0x30;
    memory[5] = 0x00;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("CALL NC", testOutput, z80, &expected) ||
        !testInt("CALL NC run result", testOutput, run, 3))
    {
        return 0;
    }
    
    z80->f = 0;
    expected.f = 0;
    expected.pc = 0x0030;
    expected.sp = 0x8020;
    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("CALL NC branch", testOutput, z80, &expected) &&
        testInt("CALL NC branch run result", testOutput, run, 5) &&
        testInt("CALL NC saved return", testOutput, memory[0x20], 0x6) &&
        testInt("CALL NC saved return", testOutput, memory[0x21], 0x0) &&
        1
    ;
}

int testPUSH_DE(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = 0;
    z80->sp = 0x8022;
    z80->d = 0x45;
    z80->e = 0x24;
    expected = *z80;
    expected.pc = 1;
    expected.sp = 0x8020;

    memory[0] = Z80_PUSH_DE;

    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("PUSH DE", testOutput, z80, &expected) &&
        testInt("PUSH DE run result", testOutput, run, 4) &&
        testInt("PUSH DE pushed value", testOutput, memory[0x20], 0x24) &&
        testInt("PUSH DE pushed value", testOutput, memory[0x21], 0x45) &&
        1
    ;
}


int testRET_C(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = 0;
    z80->sp = 0x8020;
    expected = *z80;
    expected.pc = 1;

    memory[0] = Z80_RET_C;
    memory[1] = Z80_RET_C;

    memory[0x20] = 0xE0;
    memory[0x21] = 0x34;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("RET C", testOutput, z80, &expected) ||
        !testInt("RET C run result", testOutput, run, 2))
    {
        return 0;
    }
    
    z80->f = GB_FLAGS_C;
    expected.f = GB_FLAGS_C;
    expected.pc = 0x34E0;
    expected.sp = 0x8022;
    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("RET C branch", testOutput, z80, &expected) &&
        testInt("RET C branch run result", testOutput, run, 3) &&
        1
    ;
}

int testRETI(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->sp = 0x8020;
    expected = *z80;
    expected.pc = 0x34E0;
    expected.sp = 0x8022;
    expected.interrupts = GB_INTERRUPTS_ENABLED;

    memory[0] = Z80_RETI;

    memory[0x20] = 0xE0;
    memory[0x21] = 0x34;

    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("RETI branch", testOutput, z80, &expected) &&
        testInt("RETI branch run result", testOutput, run, 3) &&
        1
    ;
}

int testJP_C(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = 0;
    expected = *z80;
    expected.pc = 3;

    memory[0] = Z80_JP_C_a16;
    memory[1] = 0x20;
    memory[2] = 0x00;
    memory[3] = Z80_JP_C_a16;
    memory[4] = 0x30;
    memory[5] = 0x00;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("JP C", testOutput, z80, &expected) ||
        !testInt("JP C run result", testOutput, run, 3))
    {
        return 0;
    }
    
    z80->f = GB_FLAGS_C;
    expected.f = GB_FLAGS_C;
    expected.pc = 0x0030;
    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("JP C branch", testOutput, z80, &expected) &&
        testInt("JP C branch run result", testOutput, run, 3) &&
        1
    ;
}

int testCALL_C(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = 0;
    z80->sp = 0x8022;
    expected = *z80;
    expected.pc = 3;

    memory[0] = Z80_CALL_C;
    memory[1] = 0x20;
    memory[2] = 0x00;
    memory[3] = Z80_CALL_C;
    memory[4] = 0x30;
    memory[5] = 0x00;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("CALL C", testOutput, z80, &expected) ||
        !testInt("CALL C run result", testOutput, run, 3))
    {
        return 0;
    }
    
    z80->f = GB_FLAGS_C;
    expected.f = GB_FLAGS_C;
    expected.pc = 0x0030;
    expected.sp = 0x8020;
    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("CALL C branch", testOutput, z80, &expected) &&
        testInt("CALL C branch run result", testOutput, run, 5) &&
        testInt("CALL C saved return", testOutput, memory[0x20], 0x6) &&
        testInt("CALL C saved return", testOutput, memory[0x21], 0x0) &&
        1
    ;
}

int run0xDTests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    return 
        testRET_NC(z80, memoryMap, memory, testOutput) &&
        testPOP_DE(z80, memoryMap, memory, testOutput) &&
        testJP_NC(z80, memoryMap, memory, testOutput) &&
        testCALL_NC(z80, memoryMap, memory, testOutput) &&
        testPUSH_DE(z80, memoryMap, memory, testOutput) &&
        testSingleADD(z80, memoryMap, memory, testOutput, d8_REGISTER_INDEX, Z80_SUB_A_d8, 0) &&
        testSingleADD(z80, memoryMap, memory, testOutput, d8_REGISTER_INDEX, Z80_SUB_A_d8, 1) &&
        testRST(z80, memoryMap, memory, testOutput, Z80_RST_10H, 0x10) &&
        testRET_C(z80, memoryMap, memory, testOutput) &&
        testRETI(z80, memoryMap, memory, testOutput) &&
        testJP_C(z80, memoryMap, memory, testOutput) &&
        testCALL_C(z80, memoryMap, memory, testOutput) &&
        testSingleADD(z80, memoryMap, memory, testOutput, d8_REGISTER_INDEX, Z80_SBC_A_d8, 0) &&
        testSingleADD(z80, memoryMap, memory, testOutput, d8_REGISTER_INDEX, Z80_SBC_A_d8, 1) &&
        testRST(z80, memoryMap, memory, testOutput, Z80_RST_18H, 0x18) &&
        1
    ;
}