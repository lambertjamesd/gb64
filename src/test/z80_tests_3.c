
#include "z80_test.h"

int testJR_NC(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 4;

    memory[0] = Z80_JR_NC;
    memory[1] = 2;
    memory[2] = Z80_NOP;
    memory[3] = Z80_NOP;
    memory[4] = Z80_JR_NC;
    memory[5] = (unsigned char)(-6);

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("JR NC", testOutput, z80, &expected) ||
        !testInt("JR NC run result", testOutput, run, 3))
    {
        return 0;
    }
    
    expected.pc = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    
    if (!testZ80State("JR NC", testOutput, z80, &expected) ||
        !testInt("JR NC run result", testOutput, run, 3))
    {
        return 0;
    }

    z80->f = GB_FLAGS_C;
    expected.pc = 2;
    expected.f = GB_FLAGS_C;
    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("JR NC", testOutput, z80, &expected) &&
        testInt("JR NC run result", testOutput, run, 2) &&
        1
    ;
}

int testLD_SP_d16(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 3;
    expected.sp = 0xD011;

    memory[0] = Z80_LD_SP_d16;
    memory[1] = 0x11;
    memory[2] = 0xD0;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("LD SP d16", testOutput, z80, &expected) &&
        testInt("LD SP d16 run result", testOutput, run, 3) &&
        1
    ;
}

int testLDD_HL_A(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);

    z80->h = 0x81;
    z80->l = 0x05;
    z80->a = 63;

    expected = *z80;
    expected.pc = 1;
    expected.l = 0x04;

    memory[0] = Z80_LDD_HL_A;
    memory[1] = Z80_LDD_HL_A;

    run = runZ80CPU(z80, memoryMap, 1);
    
    if (
        !testZ80State("LDD HL A", testOutput, z80, &expected) ||
        !testInt("LDD HL A run result", testOutput, run, 2) ||
        !testInt("LDD HL A stored value", testOutput, memory[0x105], 63))
    {
        return 0;
    }

    z80->l = 0x00;
    run = runZ80CPU(z80, memoryMap, 1);
    expected.pc = 2;
    expected.h = 0x80;
    expected.l = 0xFF;

    return 
        testZ80State("LDD HL A", testOutput, z80, &expected) &&
        testInt("LDD HL A run result", testOutput, run, 2) &&
        testInt("LDD HL A stored value", testOutput, memory[0x100], 63) &&
        1
    ;
}

int testINC_SP(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);

    z80->sp = 0x432;

    expected = *z80;
    expected.pc = 1;
    expected.sp = 0x433;

    memory[0] = Z80_INC_SP;
    memory[1] = Z80_INC_SP;

    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("INC SP", testOutput, z80, &expected) &&
        testInt("INC SP run result", testOutput, run, 2);
}

int testINC_HL_ADDR(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->h = 0x81;
    z80->l = 0x30;
    expected = *z80;
    expected.pc = 1;

    memory[0] = Z80_INC_HL_ADDR;
    memory[0x130] = 0x31;

    run = runZ80CPU(z80, memoryMap, 1);
    
    return 
        testZ80State("INC (HL) half carry", testOutput, z80, &expected) &&
        testInt("INC (HL) run result", testOutput, run, 3) &&
        testInt("INC (HL) memory", testOutput, memory[0x130], 0x32) &&
        1
    ;
}

int testDEC_HL_ADDR(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->h = 0x81;
    z80->l = 0x30;
    expected = *z80;
    expected.pc = 1;
    expected.f = 0x60;

    memory[0] = Z80_DEC_HL_ADDR;
    memory[0x130] = 0x31;

    run = runZ80CPU(z80, memoryMap, 1);
    
    return 
        testZ80State("DEC (HL) half carry", testOutput, z80, &expected) &&
        testInt("DEC (HL) run result", testOutput, run, 3) &&
        testInt("DEC (HL) memory", testOutput, memory[0x130], 0x30) &&
        1
    ;
}

int testLD_HL_d8(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->h = 0x81;
    z80->l = 0x24;
    expected = *z80;
    expected.pc = 2;

    memory[0] = Z80_LD_HL_d8;
    memory[1] = 23;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("LD (HL) d8", testOutput, z80, &expected) &&
        testInt("LD (HL) d8 run result", testOutput, run, 3) &&
        testInt("LD (HL) d8 memory", testOutput, memory[0x124], 23) &&
        1
    ;
}


int testSCF(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = 0x80;
    expected = *z80;
    expected.pc = 1;
    expected.f = 0x90;

    memory[0] = Z80_SCF;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("SCF", testOutput, z80, &expected) &&
        testInt("SCF run result", testOutput, run, 1) &&
        1
    ;
}

int testJR_C(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = GB_FLAGS_C;
    expected = *z80;
    expected.pc = 4;

    memory[0] = Z80_JR_C;
    memory[1] = 2;
    memory[2] = Z80_NOP;
    memory[3] = Z80_NOP;
    memory[4] = Z80_JR_C;
    memory[5] = (unsigned char)(-6);

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("JR C", testOutput, z80, &expected) ||
        !testInt("JR C run result", testOutput, run, 3))
    {
        return 0;
    }
    
    expected.pc = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    
    if (!testZ80State("JR C second", testOutput, z80, &expected) ||
        !testInt("JR C run result", testOutput, run, 3))
    {
        return 0;
    }

    z80->f = 0;
    expected.pc = 2;
    expected.f = 0;
    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("JR C third", testOutput, z80, &expected) &&
        testInt("JR C run result", testOutput, run, 2) &&
        1
    ;
}

int testADD_HL_SP(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->h = 0x02;
    z80->l = 0x80;
    z80->sp = 0x8081;
    expected = *z80;
    expected.pc = 1;
    expected.h = 0x83;
    expected.l = 0x01;
    expected.f = 0x0;

    memory[0] = Z80_ADD_HL_SP;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("HL SP half", testOutput, z80, &expected) &&
        testInt("HL SP run result", testOutput, run, 2) &&
        1
    ;
}

int testLDD_A_HL(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->h = 0x21;
    z80->l = 0x00;
    expected = *z80;
    expected.pc = 1;
    expected.a = 0xA3;
    expected.h = 0x20;
    expected.l = 0xFF;

    memory[0] = Z80_LDD_A_HL;
    memory[0x100] = 0xA3;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("LDD A HL", testOutput, z80, &expected) &&
        testInt("LDD A HL run result", testOutput, run, 2) &&
        1
    ;
}


int testDEC_SP(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);

    z80->sp = 0x1234;

    expected = *z80;
    expected.pc = 1;
    expected.sp = 0x1233;

    memory[0] = Z80_DEC_SP;

    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("DEC SP", testOutput, z80, &expected) &&
        testInt("DEC SP run result", testOutput, run, 2);
}


int testINC_A(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 1;
    expected.a = 1;

    memory[0] = Z80_INC_A;

    run = runZ80CPU(z80, memoryMap, 1);
    
    return 
        testZ80State("INC A", testOutput, z80, &expected) &&
        testInt("INC A", testOutput, run, 1) &&
        1
    ;
}

int testDEC_A(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.f = 0x40;
    expected.pc = 1;
    expected.a = 0xFF;

    memory[0] = Z80_DEC_A;

    run = runZ80CPU(z80, memoryMap, 1);
    
    return 
        testZ80State("DEC A", testOutput, z80, &expected) &
        testInt("DEC A", testOutput, run, 1) &&
        1
    ;
}


int testLD_A_d8(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 2;
    expected.a = 23;

    memory[0] = Z80_LD_A_d8;
    memory[1] = 23;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("LD A d8", testOutput, z80, &expected) &&
        testInt("LD A d8 run result", testOutput, run, 2) &&
        1
    ;
}

int testCCF(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 1;
    z80->f = 0x80;
    expected.f = 0x90;

    memory[0] = Z80_CCF;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("CCF d16", testOutput, z80, &expected) &&
        testInt("CCF run result", testOutput, run, 1) &&
        1
    ;
}

int run0x3Tests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    return 
        testJR_NC(z80, memoryMap, memory, testOutput) &&
        testLD_SP_d16(z80, memoryMap, memory, testOutput) &&
        testLDD_HL_A(z80, memoryMap, memory, testOutput) &&
        testINC_SP(z80, memoryMap, memory, testOutput) &&
        testINC_HL_ADDR(z80, memoryMap, memory, testOutput) &&
        testDEC_HL_ADDR(z80, memoryMap, memory, testOutput) &&
        testLD_HL_d8(z80, memoryMap, memory, testOutput) &&
        testSCF(z80, memoryMap, memory, testOutput) &&
        testJR_C(z80, memoryMap, memory, testOutput) &&
        testADD_HL_SP(z80, memoryMap, memory, testOutput) &&
        testLDD_A_HL(z80, memoryMap, memory, testOutput) &&
        testDEC_SP(z80, memoryMap, memory, testOutput) &&
        testINC_A(z80, memoryMap, memory, testOutput) &&
        testDEC_A(z80, memoryMap, memory, testOutput) &&
        testLD_A_d8(z80, memoryMap, memory, testOutput) &&
        testCCF(z80, memoryMap, memory, testOutput) &&
        1
    ;
}