
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

    return testZ80State("LDH a8 A branch", testOutput, z80, &expected) &&
        testInt("LDH a8 A branch run result", testOutput, run, 3) &&
        testInt("LDH a8 A store result", testOutput, memory->miscBytes[0x1F0], 0x3E) &&
        1
    ;
}

int testPOP_HL(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->sp = 0x8020;
    expected = *z80;
    expected.pc = 1;
    expected.sp = 0x8022;
    expected.h = 0x34;
    expected.l = 0xE0;

    memory->internalRam[0] = Z80_POP_HL;

    memory->internalRam[0x20] = 0xE0;
    memory->internalRam[0x21] = 0x34;

    run = runZ80CPU(z80, memory, 1);

    return testZ80State("POP HL branch", testOutput, z80, &expected) &&
        testInt("POP HL branch run result", testOutput, run, 3) &&
        1
    ;
}

int testLDH_C_ADDR_A(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->a = 0x3E;
    z80->c = 0xF0;
    expected = *z80;
    expected.pc = 1;

    memory->internalRam[0] = Z80_LD_C_ADDR_A;

    run = runZ80CPU(z80, memory, 1);

    return testZ80State("LD (C) A branch", testOutput, z80, &expected) &&
        testInt("LD (C) A branch run result", testOutput, run, 2) &&
        testInt("LD (C) A store result", testOutput, memory->miscBytes[0x1F0], 0x3E) &&
        1
    ;
}

int testPUSH_HL(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = 0;
    z80->sp = 0x8022;
    z80->h = 0x45;
    z80->l = 0x24;
    expected = *z80;
    expected.pc = 1;
    expected.sp = 0x8020;

    memory->internalRam[0] = Z80_PUSH_HL;

    run = runZ80CPU(z80, memory, 1);

    return testZ80State("PUSH HL", testOutput, z80, &expected) &&
        testInt("PUSH HL run result", testOutput, run, 4) &&
        testInt("PUSH HL pushed value", testOutput, memory->internalRam[0x20], 0x24) &&
        testInt("PUSH HL pushed value", testOutput, memory->internalRam[0x21], 0x45) &&
        1
    ;
}

int testADD_SP_r8(struct Z80State* z80, struct Memory* memory, char* testOutput)
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
    expected.sp = 0x8040;

    memory->internalRam[0] = Z80_ADD_SP_r8;
    memory->internalRam[1] = 0x20;
    memory->internalRam[2] = Z80_ADD_SP_r8;
    memory->internalRam[3] = -0x30;

    run = runZ80CPU(z80, memory, 1);

    if (
        !testZ80State("ADD SP r8 HL", testOutput, z80, &expected) ||
        !testInt("ADD SP r8 run result", testOutput, run, 4)
    )
    {
        return 0;
    }

    run = runZ80CPU(z80, memory, 1);
    expected.pc = 4;
    expected.sp = 0x8010;
    expected.f = GB_FLAGS_H;

    return testZ80State("ADD SP r8 HL", testOutput, z80, &expected) &&
        testInt("ADD SP r8 run result", testOutput, run, 4) &&
        1
    ;
}

int testJP_HL(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->h = 0x45;
    z80->l = 0x24;
    expected = *z80;
    expected.pc = 0x4524;

    memory->internalRam[0] = Z80_JP_HL;

    run = runZ80CPU(z80, memory, 1);

    return testZ80State("JP HL", testOutput, z80, &expected) &&
        testInt("JP HL run result", testOutput, run, 1) &&
        1
    ;
}

int testLD_a16_A(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->a = 0x72;
    expected = *z80;
    expected.pc = 3;

    memory->internalRam[0] = Z80_LD_a16_A;
    memory->internalRam[1] = 0x40;
    memory->internalRam[2] = 0x80;
    memory->internalRam[0x40] = 0x72;

    run = runZ80CPU(z80, memory, 1);

    return testZ80State("LD a16 A branch", testOutput, z80, &expected) &&
        testInt("LD a16 A branch run result", testOutput, run, 4) &&
        1
    ;
}

int run0xETests(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    return 
        testLDH_a8_A(z80, memory, testOutput) &&
        testPOP_HL(z80, memory, testOutput) &&
        testLDH_C_ADDR_A(z80, memory, testOutput) &&
        testPUSH_HL(z80, memory, testOutput) &&
        testSingleBitwise( z80, memory, testOutput, d8_REGISTER_INDEX, Z80_AND_A_d8) &&
        testRST(z80, memory, testOutput, Z80_RST_20H, 0x20) &&
        testADD_SP_r8(z80, memory, testOutput) && 
        testJP_HL(z80, memory, testOutput) && 
        testLD_a16_A(z80, memory, testOutput) &&
        testSingleBitwise( z80, memory, testOutput, d8_REGISTER_INDEX, Z80_XOR_A_d8) &&
        testRST(z80, memory, testOutput, Z80_RST_28H, 0x28) &&
        1
    ;
}