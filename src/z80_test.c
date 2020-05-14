#include "z80.h"
#include "z80_test.h"
#include "memory_map.h"

int testInt(
    char *testName,
    char *testOutput,
    int actual,
    int expected
) {
    if (actual != expected)
    {
        sprintf(testOutput, "Failed %s:\n E %d A %d", testName, expected, actual);
        return 0;
    }

    return 1;
}

int testZ80State(
    char *testName,
    char *testOutput,
    struct Z80State* actual,
    struct Z80State* expected
) {
    if (actual->a != expected->a || actual->f != expected->f || actual->b != expected->b || actual->c != expected->c ||
        actual->d != expected->d || actual->e != expected->e || actual->h != expected->h || actual->l != expected->l ||
        actual->pc != expected->pc || actual->sp != expected->sp)
    {
        sprintf(testOutput, 
            "Failed z80: %s\n   A  F  B  C  D  E  H  L  PC   SP\n E %02X %02X %02X %02X %02X %02X %02X %02X %04X %04X\n A %02X %02X %02X %02X %02X %02X %02X %02X %04X %04X",
            testName, 
            expected->a, expected->f, expected->b, expected->c, 
            expected->d, expected->e, expected->h, expected->l, 
            expected->pc, expected->sp,
            actual->a, actual->f, actual->b, actual->c, 
            actual->d, actual->e, actual->h, actual->l, 
            actual->pc, actual->sp
        );

        return 0;
    }

    return 1;
}

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

int runTests(char* testOutput) {
    struct Z80State z80;
    unsigned char memory[512];
    void* memoryMap[MEMORY_MAP_SIZE];
    int i;
    char subTestOutput[200];

    for (i = 0; i < MEMORY_MAP_SIZE; ++i)
    {
        memoryMap[i] = memory;
    }

    zeroMemory(memory, 512);
    initializeZ80(&z80);

    if (
        !testNOP(&z80, memoryMap, memory, subTestOutput) ||
        !testLD_BC_d16(&z80, memoryMap, memory, subTestOutput) ||
        !testLD_BC_A(&z80, memoryMap, memory, subTestOutput) ||
        !testINC_BC(&z80, memoryMap, memory, subTestOutput) ||
        !testINC_B(&z80, memoryMap, memory, subTestOutput) ||
        !testDEC_B(&z80, memoryMap, memory, subTestOutput) ||
        !testLD_B_d8(&z80, memoryMap, memory, subTestOutput) ||
        !testRLCA(&z80, memoryMap, memory, subTestOutput) ||
        !testLD_A16_SP(&z80, memoryMap, memory, subTestOutput) ||
        0)
    {
        sprintf(testOutput, "runZ80CPU 0x%X\n%s", &runZ80CPU, subTestOutput);
        return 0;
    }

	sprintf(testOutput, "Tests Passed");

    return 1;
}