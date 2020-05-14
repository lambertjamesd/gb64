
#include "z80_test.h"

int testSTOP(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 2;
    expected.stopReason = STOP_REASON_STOP;

    memory[0] = Z80_STOP;

    run = runZ80CPU(z80, memoryMap, 2); // try to run two instructions

    return 
        testZ80State("STOP", testOutput, z80, &expected) &&
        testInt("STOP run result", testOutput, run, 1) &&
        1
    ;
}

int testLD_DE_d16(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 3;
    expected.d = 13;
    expected.e = 17;

    memory[0] = Z80_LD_DE_d16;
    memory[1] = 13;
    memory[2] = 17;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("LD DE d16", testOutput, z80, &expected) &&
        testInt("LD DE d16 run result", testOutput, run, 3) &&
        1
    ;
}

int testLD_DE_A(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);

    z80->d = 0x81;
    z80->e = 0x05;
    z80->a = 63;

    expected = *z80;
    expected.pc = 1;

    memory[0] = Z80_LD_DE_A;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("LD DE A", testOutput, z80, &expected) &&
        testInt("LD DE A run result", testOutput, run, 2) &&
        testInt("LD DE A stored value", testOutput, memory[0x105], 63) &&
        1
    ;
}

int testINC_DE(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);

    z80->d = 1;
    z80->e = 3;

    expected = *z80;
    expected.pc = 1;
    expected.d = 1;
    expected.e = 4;

    memory[0] = Z80_INC_DE;
    memory[1] = Z80_INC_DE;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("INC DE", testOutput, z80, &expected) ||
        !testInt("INC DE run result", testOutput, run, 2))
    {
        return 0;
    }

    z80->d = 0; z80->e = 0xFF;
    expected.d = 1;
    expected.e = 0;
    expected.pc = 2;
    
    run = runZ80CPU(z80, memoryMap, 1);

    return testZ80State("INC DE rollover", testOutput, z80, &expected);
}

int testINC_D(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 1;
    expected.d = 1;

    memory[0] = Z80_INC_D;
    memory[1] = Z80_INC_D;
    memory[2] = Z80_INC_D;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("INC D", testOutput, z80, &expected) ||
        !testInt("INC D run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting zero flag
    z80->d = 0xFF;
    z80->f = 0x0;
    expected.f = 0xA0;
    expected.pc = 2;
    expected.d = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    
    if (!testZ80State("INC D overflow", testOutput, z80, &expected) ||
        !testInt("INC D run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting half flag
    z80->d = 0x1F;
    z80->f = 0;
    expected.f = 0x20;
    expected.pc = 3;
    expected.d = 0x20;
    run = runZ80CPU(z80, memoryMap, 1);
    
    return 
        testZ80State("INC D half carry", testOutput, z80, &expected) &&
        testInt("INC D run result", testOutput, run, 1) &&
        1
    ;
}

int testDEC_D(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.f = 0x40;
    expected.pc = 1;
    expected.d = 0xFF;

    memory[0] = Z80_DEC_D;
    memory[1] = Z80_DEC_D;
    memory[2] = Z80_DEC_D;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("DEC D", testOutput, z80, &expected) ||
        !testInt("DEC D run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting zero flag
    z80->d = 0x1;
    z80->f = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    expected.f = 0xC0;
    expected.pc = 2;
    expected.d = 0;
    
    if (!testZ80State("DEC D zero", testOutput, z80, &expected) ||
        !testInt("DEC D zero run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting half flag
    z80->d = 0x10;
    z80->f = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    expected.f = 0x40;
    expected.pc = 3;
    expected.d = 0x0F;
    
    return 
        testZ80State("DEC D half carry", testOutput, z80, &expected) &
        testInt("DEC D half carry run result", testOutput, run, 1) &&
        1
    ;
}

int testLD_D_d8(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 2;
    expected.d = 23;

    memory[0] = Z80_LD_D_d8;
    memory[1] = 23;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("LD D d8", testOutput, z80, &expected) &&
        testInt("LD D d8 run result", testOutput, run, 2) &&
        1
    ;
}

int testRLA(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 1;
    expected.a = 0x3;
    z80->a = 0x1;
    z80->f = 0xF0;

    memory[0] = Z80_RLA;
    memory[1] = Z80_RLA;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("RLA", testOutput, z80, &expected) ||
        !testInt("RLA run result", testOutput, run, 1))
    {
        return 0;
    }

    z80->a = 0x80;
    expected.pc = 2;
    expected.a = 0x0;
    expected.f = 0x90;
    run = runZ80CPU(z80, memoryMap, 1);
    
    return 
        testZ80State("RLA set carry", testOutput, z80, &expected) &&
        testInt("RLA run result", testOutput, run, 1) &&
        1
    ;
}

int testJA(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 4;

    memory[0] = Z80_JA;
    memory[1] = 2;
    memory[2] = Z80_NOP;
    memory[3] = Z80_NOP;
    memory[4] = Z80_JA;
    memory[5] = (unsigned char)(-6);

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("JA", testOutput, z80, &expected) ||
        !testInt("JA run result", testOutput, run, 3))
    {
        return 0;
    }
    
    expected.pc = 0;
    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("JA", testOutput, z80, &expected) &&
        testInt("JA run result", testOutput, run, 3) &&
        1
    ;
}

int run0x1Tests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    return 
        testSTOP(z80, memoryMap, memory, testOutput) &&
        testLD_DE_d16(z80, memoryMap, memory, testOutput) &&
        testLD_DE_A(z80, memoryMap, memory, testOutput) &&
        testINC_DE(z80, memoryMap, memory, testOutput) &&
        testINC_D(z80, memoryMap, memory, testOutput) &&
        testDEC_D(z80, memoryMap, memory, testOutput) &&
        testLD_D_d8(z80, memoryMap, memory, testOutput) &&
        testRLA(z80, memoryMap, memory, testOutput) &&
        testJA(z80, memoryMap, memory, testOutput) &&
        1
    ;
}