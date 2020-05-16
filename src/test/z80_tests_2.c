
#include "z80_test.h"

int testJR_NZ(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 4;

    memory[0] = Z80_JR_NZ;
    memory[1] = 2;
    memory[2] = Z80_NOP;
    memory[3] = Z80_NOP;
    memory[4] = Z80_JR_NZ;
    memory[5] = (unsigned char)(-6);

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("JR NZ", testOutput, z80, &expected) ||
        !testInt("JR NZ run result", testOutput, run, 3))
    {
        return 0;
    }
    
    expected.pc = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    
    if (!testZ80State("JR NZ", testOutput, z80, &expected) ||
        !testInt("JR NZ run result", testOutput, run, 3))
    {
        return 0;
    }

    z80->f = GB_FLAGS_Z;
    expected.pc = 2;
    expected.f = GB_FLAGS_Z;
    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("JR NZ", testOutput, z80, &expected) &&
        testInt("JR NZ run result", testOutput, run, 2) &&
        1
    ;
}

int testLD_HL_d16(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 3;
    expected.h = 13;
    expected.l = 17;

    memory[0] = Z80_LD_HL_d16;
    memory[1] = 13;
    memory[2] = 17;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("LD HL d16", testOutput, z80, &expected) &&
        testInt("LD HL d16 run result", testOutput, run, 3) &&
        1
    ;
}

int testLDI_HL_A(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);

    z80->h = 0x81;
    z80->l = 0x05;
    z80->a = 63;

    expected = *z80;
    expected.pc = 1;
    expected.l = 0x06;

    memory[0] = Z80_LDI_HL_A;
    memory[1] = Z80_LDI_HL_A;

    run = runZ80CPU(z80, memoryMap, 1);
    
    if (
        !testZ80State("LDI HL A", testOutput, z80, &expected) ||
        !testInt("LDI HL A run result", testOutput, run, 2) ||
        !testInt("LDI HL A stored value", testOutput, memory[0x105], 63))
    {
        return 0;
    }

    z80->l = 0xFF;
    run = runZ80CPU(z80, memoryMap, 1);
    expected.pc = 2;
    expected.h = 0x82;
    expected.l = 0x00;

    return 
        testZ80State("LD HL A", testOutput, z80, &expected) &&
        testInt("LD HL A run result", testOutput, run, 2) &&
        testInt("LD HL A stored value", testOutput, memory[0x1FF], 63) &&
        1
    ;
}

int testINC_HL(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);

    z80->h = 1;
    z80->l = 3;

    expected = *z80;
    expected.pc = 1;
    expected.h = 1;
    expected.l = 4;

    memory[0] = Z80_INC_HL;
    memory[1] = Z80_INC_HL;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("INC HL", testOutput, z80, &expected) ||
        !testInt("INC HL run result", testOutput, run, 2))
    {
        return 0;
    }

    z80->h = 0; z80->l = 0xFF;
    expected.h = 1;
    expected.l = 0;
    expected.pc = 2;
    
    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("INC HL rollover", testOutput, z80, &expected);
}

int testINC_H(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 1;
    expected.h = 1;

    memory[0] = Z80_INC_H;
    memory[1] = Z80_INC_H;
    memory[2] = Z80_INC_H;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("INC H", testOutput, z80, &expected) ||
        !testInt("INC H run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting zero flag
    z80->h = 0xFF;
    z80->f = 0x0;
    expected.f = 0xA0;
    expected.pc = 2;
    expected.h = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    
    if (!testZ80State("INC H overflow", testOutput, z80, &expected) ||
        !testInt("INC H run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting half flag
    z80->h = 0x1F;
    z80->f = 0;
    expected.f = 0x20;
    expected.pc = 3;
    expected.h = 0x20;
    run = runZ80CPU(z80, memoryMap, 1);
    
    return 
        testZ80State("INC H half carry", testOutput, z80, &expected) &&
        testInt("INC H run result", testOutput, run, 1) &&
        1
    ;
}

int testDEC_H(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.f = 0x40;
    expected.pc = 1;
    expected.h = 0xFF;

    memory[0] = Z80_DEC_H;
    memory[1] = Z80_DEC_H;
    memory[2] = Z80_DEC_H;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("DEC H", testOutput, z80, &expected) ||
        !testInt("DEC H run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting zero flag
    z80->h = 0x1;
    z80->f = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    expected.f = 0xC0;
    expected.pc = 2;
    expected.h = 0;
    
    if (!testZ80State("DEC H zero", testOutput, z80, &expected) ||
        !testInt("DEC H zero run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting half flag
    z80->h = 0x10;
    z80->f = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    expected.f = 0x40;
    expected.pc = 3;
    expected.h = 0x0F;
    
    return 
        testZ80State("DEC H half carry", testOutput, z80, &expected) &
        testInt("DEC H half carry run result", testOutput, run, 1) &&
        1
    ;
}

int testLD_H_d8(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 2;
    expected.h = 23;

    memory[0] = Z80_LD_H_d8;
    memory[1] = 23;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("LD H d8", testOutput, z80, &expected) &&
        testInt("LD H d8 run result", testOutput, run, 2) &&
        1
    ;
}


int testDAA(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    return 1; // TODO
}

int testJR_Z(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = GB_FLAGS_Z;
    expected = *z80;
    expected.pc = 4;

    memory[0] = Z80_JR_Z;
    memory[1] = 2;
    memory[2] = Z80_NOP;
    memory[3] = Z80_NOP;
    memory[4] = Z80_JR_Z;
    memory[5] = (unsigned char)(-6);

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("JR Z", testOutput, z80, &expected) ||
        !testInt("JR NZ run result", testOutput, run, 3))
    {
        return 0;
    }
    
    expected.pc = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    
    if (!testZ80State("JR Z second", testOutput, z80, &expected) ||
        !testInt("JR Z run result", testOutput, run, 3))
    {
        return 0;
    }

    z80->f = 0;
    expected.pc = 2;
    expected.f = 0;
    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("JR NZ third", testOutput, z80, &expected) &&
        testInt("JR NZ run result", testOutput, run, 2) &&
        1
    ;
}

int testADD_HL_HL(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->h = 0x02;
    z80->l = 0x80;
    expected = *z80;
    expected.pc = 1;
    expected.h = 0x05;
    expected.l = 0x00;
    expected.f = 0x0;

    memory[0] = Z80_ADD_HL_HL;
    
    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("HL HL", testOutput, z80, &expected) &&
        testInt("HL HL run result", testOutput, run, 2) &&
        1
    ;
}

int testLDI_A_HL(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->h = 0x21;
    z80->l = 0xFF;
    expected = *z80;
    expected.pc = 1;
    expected.a = 0xA3;
    expected.h = 0x22;
    expected.l = 0x00;

    memory[0] = Z80_LDI_A_HL;
    memory[0x1FF] = 0xA3;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("LDI A HL", testOutput, z80, &expected) &&
        testInt("LDI A HL run result", testOutput, run, 2) &&
        1
    ;
}


int testDEC_HL(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);

    z80->h = 1;
    z80->l = 3;

    expected = *z80;
    expected.pc = 1;
    expected.h = 1;
    expected.l = 2;

    memory[0] = Z80_DEC_HL;
    memory[1] = Z80_DEC_HL;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("DEC HL", testOutput, z80, &expected) ||
        !testInt("DEC HL run result", testOutput, run, 2))
    {
        return 0;
    }

    z80->h = 1; z80->l = 0;
    expected.h = 0;
    expected.l = 0xFF;
    expected.pc = 2;
    
    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("DEC HL rollover", testOutput, z80, &expected);
}


int testINC_L(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 1;
    expected.l = 1;

    memory[0] = Z80_INC_L;

    run = runZ80CPU(z80, memoryMap, 1);
    
    return 
        testZ80State("INC L", testOutput, z80, &expected) &&
        testInt("INC L", testOutput, run, 1) &&
        1
    ;
}

int testDEC_L(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.f = 0x40;
    expected.pc = 1;
    expected.l = 0xFF;

    memory[0] = Z80_DEC_L;

    run = runZ80CPU(z80, memoryMap, 1);
    
    return 
        testZ80State("DEC E", testOutput, z80, &expected) &
        testInt("DEC E", testOutput, run, 1) &&
        1
    ;
}


int testLD_L_d8(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 2;
    expected.l = 23;

    memory[0] = Z80_LD_L_d8;
    memory[1] = 23;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("LD E d8", testOutput, z80, &expected) &&
        testInt("LD E d8 run result", testOutput, run, 2) &&
        1
    ;
}
int testCPL(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 1;
    z80->a = 0x3E;
    expected.a = ~0x3E;
    expected.f = 0x60;

    memory[0] = Z80_CPL;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("CPL d16", testOutput, z80, &expected) &&
        testInt("CPL run result", testOutput, run, 1) &&
        1
    ;
}

int run0x2Tests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    return 
        testJR_NZ(z80, memoryMap, memory, testOutput) &&
        testLD_HL_d16(z80, memoryMap, memory, testOutput) &&
        testLDI_HL_A(z80, memoryMap, memory, testOutput) &&
        testINC_HL(z80, memoryMap, memory, testOutput) &&
        testINC_H(z80, memoryMap, memory, testOutput) &&
        testDEC_H(z80, memoryMap, memory, testOutput) &&
        testLD_H_d8(z80, memoryMap, memory, testOutput) &&
        testDAA(z80, memoryMap, memory, testOutput) &&
        testJR_Z(z80, memoryMap, memory, testOutput) &&
        testADD_HL_HL(z80, memoryMap, memory, testOutput) &&
        testLDI_A_HL(z80, memoryMap, memory, testOutput) &&
        testDEC_HL(z80, memoryMap, memory, testOutput) &&
        testINC_L(z80, memoryMap, memory, testOutput) &&
        testDEC_L(z80, memoryMap, memory, testOutput) &&
        testLD_L_d8(z80, memoryMap, memory, testOutput) &&
        testCPL(z80, memoryMap, memory, testOutput) &&
        1
    ;
}