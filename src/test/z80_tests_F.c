
#include "z80_test.h"

int testLDH_A_a8(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 2;
    expected.a = 0x3E;

    memory->internalRam[0] = Z80_LDH_A_a8;
    memory->internalRam[1] = 0xF0;
    memory->miscBytes[0x1F0] = 0x3E;

    run = runZ80CPU(z80, memory, 1);

    return testZ80State("LDH A a8 branch", testOutput, z80, &expected) &&
        testInt("LDH A a8 branch run result", testOutput, run, 3) &&
        1
    ;
}

int testPOP_AF(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->sp = 0x8020;
    expected = *z80;
    expected.pc = 1;
    expected.sp = 0x8022;
    expected.a = 0x34;
    expected.f = 0xE0;

    memory->internalRam[0] = Z80_POP_AF;

    memory->internalRam[0x20] = 0xE2;
    memory->internalRam[0x21] = 0x34;

    run = runZ80CPU(z80, memory, 1);

    return testZ80State("POP AF branch", testOutput, z80, &expected) &&
        testInt("POP AF branch run result", testOutput, run, 3) &&
        1
    ;
}

int testLDH_A_C_ADDR(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->a = 0x3E;
    z80->c = 0xF2;
    expected = *z80;
    expected.pc = 1;
    expected.a = 0x24;

    memory->internalRam[0] = Z80_LD_A_C_ADDR;
    memory->miscBytes[0x1F2] = 0x24;

    run = runZ80CPU(z80, memory, 1);

    return testZ80State("LD A (C) A branch", testOutput, z80, &expected) &&
        testInt("LD A (C) A branch run result", testOutput, run, 2) &&
        1
    ;
}

int testDI(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->sp = 0x8022;
    z80->a = 0x45;
    z80->f = 0x24;
    z80->interrupts = GB_INTERRUPTS_ENABLED;
    expected = *z80;
    expected.pc = 1;
    expected.interrupts = 0;

    memory->internalRam[0] = Z80_DI;

    run = runZ80CPU(z80, memory, 1);

    return testZ80State("DI", testOutput, z80, &expected) &&
        testInt("DI run result", testOutput, run, 1) &&
        1
    ;
}

int testPUSH_AF(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->sp = 0x8022;
    z80->a = 0x45;
    z80->f = 0x24;
    expected = *z80;
    expected.pc = 1;
    expected.sp = 0x8020;

    memory->internalRam[0] = Z80_PUSH_AF;

    run = runZ80CPU(z80, memory, 1);

    return testZ80State("PUSH HL", testOutput, z80, &expected) &&
        testInt("PUSH HL run result", testOutput, run, 4) &&
        testInt("PUSH HL pushed value", testOutput, memory->internalRam[0x20], 0x24) &&
        testInt("PUSH HL pushed value", testOutput, memory->internalRam[0x21], 0x45) &&
        1
    ;
}

int testLD_HL_SP_r8(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = 0;
    z80->sp = 0x8020;
    z80->h = 0x45;
    z80->l = 0x24;
    expected = *z80;
    expected.pc = 2;
    expected.sp = 0x8020;
    expected.h = 0x80;
    expected.l = 0x40;

    memory->internalRam[0] = Z80_LD_HL_SP_r8;
    memory->internalRam[1] = 0x20;
    memory->internalRam[2] = Z80_LD_HL_SP_r8;
    memory->internalRam[3] = 0xF0;

    run = runZ80CPU(z80, memory, 1);

    if (
        !testZ80State("LD HL SP r8", testOutput, z80, &expected) ||
        !testInt("LD HL SP r8 run result", testOutput, run, 3)
    )
    {
        return 0;
    }

    run = runZ80CPU(z80, memory, 1);
    expected.pc = 4;
    expected.h = 0x80;
    expected.l = 0x10;

    return testZ80State("LD HL SP r8", testOutput, z80, &expected) &&
        testInt("LD HL SP r8 run result", testOutput, run, 3) &&
        1
    ;
}

int testLD_SP_HL(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->h = 0x45;
    z80->l = 0x24;
    expected = *z80;
    expected.sp = 0x4524;
    expected.pc = 1;

    memory->internalRam[0] = Z80_SP_HL;

    run = runZ80CPU(z80, memory, 1);

    return testZ80State("LD SP HL", testOutput, z80, &expected) &&
        testInt("LD SP HL run result", testOutput, run, 2) &&
        1
    ;
}

int testLD_A_a16(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.a = 0x72;
    expected.pc = 3;

    memory->internalRam[0] = Z80_LD_A_a16;
    memory->internalRam[1] = 0x40;
    memory->internalRam[2] = 0x80;
    memory->internalRam[0x40] = 0x72;

    run = runZ80CPU(z80, memory, 1);

    return testZ80State("LD a16 A", testOutput, z80, &expected) &&
        testInt("LD a16 A run result", testOutput, run, 4) &&
        1
    ;
}

int testEI(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 1;
    expected.interrupts = GB_INTERRUPTS_ENABLED;

    memory->internalRam[0] = Z80_EI;

    run = runZ80CPU(z80, memory, 1);

    return testZ80State("EI HL", testOutput, z80, &expected) &&
        testInt("EI run result", testOutput, run, 1) &&
        1
    ;
}

int run0xFTests(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    return 
        testLDH_A_a8(z80, memory, testOutput) &&
        testPOP_AF(z80, memory, testOutput) &&
        testLDH_A_C_ADDR(z80, memory, testOutput) &&
        testDI(z80, memory, testOutput) &&
        testPUSH_AF(z80, memory, testOutput) &&
        testSingleBitwise(z80, memory, testOutput, d8_REGISTER_INDEX, Z80_OR_A_d8) &&
        testRST(z80, memory, testOutput, Z80_RST_30H, 0x30) &&
        testLD_HL_SP_r8(z80, memory, testOutput) && 
        testLD_SP_HL(z80, memory, testOutput) && 
        testLD_A_a16(z80, memory, testOutput) &&
        testEI(z80, memory, testOutput) &&
        testSingleADD(z80, memory, testOutput, d8_REGISTER_INDEX, Z80_CP_A_d8, 1) &&
        testRST(z80, memory, testOutput, Z80_RST_38H, 0x38) &&
        1
    ;
}