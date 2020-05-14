#include "z80_test.h"

int testNOP(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 1;

    memory[0] = Z80_NOP;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("NOP", testOutput, z80, &expected) &&
        testInt("NOP run result", testOutput, run, 1) &&
        1
    ;
}

int testLD_BC_d16(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 3;
    expected.b = 13;
    expected.c = 17;

    memory[0] = Z80_LD_BC_d16;
    memory[1] = 13;
    memory[2] = 17;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("LD BC d16", testOutput, z80, &expected) &&
        testInt("LD BC d16 run result", testOutput, run, 3) &&
        1
    ;
}

int testLD_BC_A(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);

    z80->b = 0x81;
    z80->c = 0x05;
    z80->a = 63;

    expected = *z80;
    expected.pc = 1;

    memory[0] = Z80_LD_BC_A;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("LD BC A", testOutput, z80, &expected) &&
        testInt("LD BC A run result", testOutput, run, 2) &&
        testInt("LD BC A stored value", testOutput, memory[0x105], 63) &&
        1
    ;
}

int testINC_BC(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);

    z80->b = 1;
    z80->c = 3;

    expected = *z80;
    expected.pc = 1;
    expected.b = 1;
    expected.c = 4;

    memory[0] = Z80_INC_BC;
    memory[1] = Z80_INC_BC;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("INC BC", testOutput, z80, &expected) ||
        !testInt("INC BC run result", testOutput, run, 2))
    {
        return 0;
    }

    z80->b = 0; z80->c = 0xFF;
    expected.b = 1;
    expected.c = 0;
    expected.pc = 2;
    
    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("INC BC rollover", testOutput, z80, &expected);
}

int testINC_B(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 1;
    expected.b = 1;

    memory[0] = Z80_INC_B;
    memory[1] = Z80_INC_B;
    memory[2] = Z80_INC_B;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("INC B", testOutput, z80, &expected) ||
        !testInt("INC B run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting zero flag
    z80->b = 0xFF;
    z80->f = 0x0;
    expected.f = 0xA0;
    expected.pc = 2;
    expected.b = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    
    if (!testZ80State("INC B overflow", testOutput, z80, &expected) ||
        !testInt("INC B run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting half flag
    z80->b = 0x1F;
    z80->f = 0;
    expected.f = 0x20;
    expected.pc = 3;
    expected.b = 0x20;
    run = runZ80CPU(z80, memoryMap, 1);
    
    return 
        testZ80State("INC B half carry", testOutput, z80, &expected) &&
        testInt("INC B run result", testOutput, run, 1) &&
        1
    ;
}

int testDEC_B(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.f = 0x40;
    expected.pc = 1;
    expected.b = 0xFF;

    memory[0] = Z80_DEC_B;
    memory[1] = Z80_DEC_B;
    memory[2] = Z80_DEC_B;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("DEC B", testOutput, z80, &expected) ||
        !testInt("DEC B run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting zero flag
    z80->b = 0x1;
    z80->f = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    expected.f = 0xC0;
    expected.pc = 2;
    expected.b = 0;
    
    if (!testZ80State("DEC B zero", testOutput, z80, &expected) ||
        !testInt("DEC B zero run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting half flag
    z80->b = 0x10;
    z80->f = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    expected.f = 0x40;
    expected.pc = 3;
    expected.b = 0x0F;
    
    return 
        testZ80State("DEC B half carry", testOutput, z80, &expected) &
        testInt("DEC B half carry run result", testOutput, run, 1) &&
        1
    ;
}

int testLD_B_d8(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 2;
    expected.b = 23;

    memory[0] = Z80_LD_B_d8;
    memory[1] = 23;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("LD B d8", testOutput, z80, &expected) &&
        testInt("LD B d8 run result", testOutput, run, 2) &&
        1
    ;
}

int testRLCA(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 1;
    expected.a = 0x2;
    z80->a = 0x1;
    z80->f = 0xF0;

    memory[0] = Z80_RLCA;
    memory[1] = Z80_RLCA;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("RLCA", testOutput, z80, &expected) ||
        !testInt("RLCA run result", testOutput, run, 1))
    {
        return 0;
    }

    z80->a = 0x80;
    expected.pc = 2;
    expected.a = 0x01;
    expected.f = 0x10;
    run = runZ80CPU(z80, memoryMap, 1);
    
    return 
        testZ80State("RLCA set carry", testOutput, z80, &expected) &&
        testInt("RLCA run result", testOutput, run, 1) &&
        1
    ;
}

int testLD_A16_SP(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 3;
    expected.sp = 0xE83C;

    memory[0] = Z80_LD_a16_SP;
    memory[1] = 0xE8;
    memory[2] = 0x3C;

    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("LD a16 SP", testOutput, z80, &expected) &&
        testInt("LD a16 SP run result", testOutput, run, 5) &&
        1
    ;
}

int testADD_HL_BC(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->h = 0x02;
    z80->l = 0x80;
    z80->b = 0x80;
    z80->c = 0x81;
    expected = *z80;
    expected.pc = 1;
    expected.h = 0x83;
    expected.l = 0x01;
    expected.f = 0x0;

    memory[0] = Z80_ADD_HL_BC;
    memory[1] = Z80_ADD_HL_BC;
    memory[2] = Z80_ADD_HL_BC;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("HL BC", testOutput, z80, &expected) ||
        !testInt("HL BC run result", testOutput, run, 2))
    {
        return 0;
    }
    
    z80->b = 0xD;
    z80->c = 0;
    expected.pc = 2;
    expected.h = 0x90;
    expected.l = 0x01;
    expected.b = 0xD;
    expected.c = 0;
    expected.f = 0x20;
    
    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("HL BC", testOutput, z80, &expected) ||
        !testInt("HL BC run result", testOutput, run, 2))
    {
        return 0;
    }

    z80->b = 0x70;
    run = runZ80CPU(z80, memoryMap, 1);
    expected.pc = 3;
    expected.h = 0x00;
    expected.l = 0x01;
    expected.b = 0x70;
    expected.f = 0x10;

    return 
        testZ80State("HL BC half", testOutput, z80, &expected) &&
        testInt("HL BC run result", testOutput, run, 2) &&
        1
    ;
}

int testLD_A_BC(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->b = 0x21;
    z80->c = 0x30;
    expected = *z80;
    expected.pc = 1;
    expected.a = 0xA3;

    memory[0] = Z80_LD_A_BC;
    memory[1] = 23;
    memory[0x130] = 0xA3;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("LD A BC", testOutput, z80, &expected) &&
        testInt("LD A BC run result", testOutput, run, 2) &&
        1
    ;
}


int testDEC_BC(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);

    z80->b = 1;
    z80->c = 3;

    expected = *z80;
    expected.pc = 1;
    expected.b = 1;
    expected.c = 2;

    memory[0] = Z80_DEC_BC;
    memory[1] = Z80_DEC_BC;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("DEC BC", testOutput, z80, &expected) ||
        !testInt("DEC BC run result", testOutput, run, 2))
    {
        return 0;
    }

    z80->b = 1; z80->c = 0;
    expected.b = 0;
    expected.c = 0xFF;
    expected.pc = 2;
    
    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("DEC BC rollover", testOutput, z80, &expected);
}


int testINC_C(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 1;
    expected.c = 1;

    memory[0] = Z80_INC_C;

    run = runZ80CPU(z80, memoryMap, 1);
    
    return 
        testZ80State("INC C", testOutput, z80, &expected) &&
        testInt("INC C", testOutput, run, 1) &&
        1
    ;
}

int testDEC_C(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.f = 0x40;
    expected.pc = 1;
    expected.c = 0xFF;

    memory[0] = Z80_DEC_C;

    run = runZ80CPU(z80, memoryMap, 1);
    
    return 
        testZ80State("DEC C", testOutput, z80, &expected) &
        testInt("DEC C", testOutput, run, 1) &&
        1
    ;
}


int testLD_C_d8(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 2;
    expected.c = 23;

    memory[0] = Z80_LD_C_d8;
    memory[1] = 23;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("LD C d8", testOutput, z80, &expected) &&
        testInt("LD C d8 run result", testOutput, run, 2) &&
        1
    ;
}

int testRRCA(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    z80->f = 0xF0;
    z80->a = 0x1;
    expected = *z80;
    expected.pc = 1;
    expected.f = 0x10;
    expected.a = 0x80;

    memory[0] = Z80_RRCA;
    memory[1] = Z80_RRCA;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("RRCA", testOutput, z80, &expected) ||
        !testInt("RRCA run result", testOutput, run, 1))
    {
        return 0;
    }
    
    z80->a = 0x80;
    expected.pc = 2;
    expected.f = 0;
    expected.a = 0x40;
    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("RRCA no carry", testOutput, z80, &expected) &&
        testInt("RRCA run result", testOutput, run, 1) &&
        1
    ;
}

int run0x0Tests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    return 
        testNOP(z80, memoryMap, memory, testOutput) &&
        testLD_BC_d16(z80, memoryMap, memory, testOutput) &&
        testLD_BC_A(z80, memoryMap, memory, testOutput) &&
        testINC_BC(z80, memoryMap, memory, testOutput) &&
        testINC_B(z80, memoryMap, memory, testOutput) &&
        testDEC_B(z80, memoryMap, memory, testOutput) &&
        testLD_B_d8(z80, memoryMap, memory, testOutput) &&
        testRLCA(z80, memoryMap, memory, testOutput) &&
        testLD_A16_SP(z80, memoryMap, memory, testOutput) &&
        testADD_HL_BC(z80, memoryMap, memory, testOutput) &&
        testLD_A_BC(z80, memoryMap, memory, testOutput) &&
        testDEC_BC(z80, memoryMap, memory, testOutput) &&
        testINC_C(z80, memoryMap, memory, testOutput) &&
        testDEC_C(z80, memoryMap, memory, testOutput) &&
        testLD_C_d8(z80, memoryMap, memory, testOutput) &&
        testRRCA(z80, memoryMap, memory, testOutput) &&
        1
    ;
}