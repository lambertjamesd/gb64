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
    struct Z80State* z80,
    unsigned char a, 
    unsigned char f, 
    unsigned char b, 
    unsigned char c, 
    unsigned char d, 
    unsigned char e, 
    unsigned char h, 
    unsigned char l, 
    unsigned short pc,
    unsigned short sp
) {
    if (z80->a != a || z80->f != f || z80->b != b || z80->c != c ||
        z80->d != d || z80->e != e || z80->h != h || z80->l != l ||
        z80->pc != pc || z80->sp != sp)
    {
        sprintf(testOutput, 
            "Failed z80: %s\n   A  F  B  C  D  E  H  L  PC   SP\n E %02X %02X %02X %02X %02X %02X %02X %02X %04X %04X\n A %02X %02X %02X %02X %02X %02X %02X %02X %04X %04X",
            testName, 
            a, f, b, c, d, e, h, l, pc, sp,
            z80->a, z80->f, z80->b, z80->c, z80->d, z80->e, z80->h, z80->l, z80->pc, z80->sp
        );

        return 0;
    }

    return 1;
}

int testNOP(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;

    z80->a = 1; z80->f = 0xF0; z80->b = 2; z80->c = 3;
    z80->d = 4; z80->e = 5; z80->h = 6; z80->l = 7;
    z80->pc = 0; z80->sp = 0;

    memory[0] = Z80_NOP;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("NOP", testOutput, z80, 1, 0xF0, 2, 3, 4, 5, 6, 7, 1, 0) ||
        !testInt("NOP run result", testOutput, run, 1))
    {
        return 0;
    }
    
    return 1;
}

int testLD_BC_d16(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;

    z80->a = 1; z80->f = 0xF0; z80->b = 2; z80->c = 3;
    z80->d = 4; z80->e = 5; z80->h = 6; z80->l = 7;
    z80->pc = 0; z80->sp = 0;

    memory[0] = Z80_LD_BC_d16;
    memory[1] = 13;
    memory[2] = 17;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("LD BC d16", testOutput, z80, 1, 0xF0, 13, 17, 4, 5, 6, 7, 3, 0) ||
        !testInt("LD BC d16 run result", testOutput, run, 3))
    {
        return 0;
    }
    
    return 1;
}

int testLD_BC_A(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;

    z80->a = 63; z80->f = 0xF0; z80->b = 0x81; z80->c = 0x5;
    z80->d = 4; z80->e = 5; z80->h = 6; z80->l = 7;
    z80->pc = 0; z80->sp = 0;

    memory[0] = Z80_LD_BC_A;

    run = runZ80CPU(z80, memoryMap, 1);

    if (
        !testZ80State("LD BC A", testOutput, z80, 63, 0xF0, 0x81, 5, 4, 5, 6, 7, 1, 0) ||
        !testInt("LD BC A run result", testOutput, run, 2) ||
        !testInt("LD BC A stored value", testOutput, memory[0x105], 63))
    {
        return 0;
    }
    
    return 1;
}

int testINC_BC(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;

    z80->a = 1; z80->f = 0xF0; z80->b = 2; z80->c = 3;
    z80->d = 4; z80->e = 5; z80->h = 6; z80->l = 7;
    z80->pc = 0; z80->sp = 0;

    memory[0] = Z80_INC_BC;
    memory[1] = Z80_INC_BC;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("INC BC", testOutput, z80, 1, 0xF0, 2, 4, 4, 5, 6, 7, 1, 0) ||
        !testInt("INC BC run result", testOutput, run, 2))
    {
        return 0;
    }

    z80->b = 0; z80->c = 0xFF;
    
    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("INC BC rollover", testOutput, z80, 1, 0xF0, 1, 0, 4, 5, 6, 7, 2, 0))
    {
        return 0;
    }
    
    return 1;
}

int testINC_B(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;

    z80->a = 1; z80->f = 0xF0; z80->b = 2; z80->c = 3;
    z80->d = 4; z80->e = 5; z80->h = 6; z80->l = 7;
    z80->pc = 0; z80->sp = 0;

    memory[0] = Z80_INC_B;
    memory[1] = Z80_INC_B;
    memory[2] = Z80_INC_B;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("INC B", testOutput, z80, 1, 0x10, 3, 3, 4, 5, 6, 7, 1, 0) ||
        !testInt("INC B run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting zero flag
    z80->b = 0xFF;
    z80->f = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    
    if (!testZ80State("INC B overflow", testOutput, z80, 1, 0xA0, 0, 3, 4, 5, 6, 7, 2, 0) ||
        !testInt("INC B run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting half flag
    z80->b = 0x1F;
    z80->f = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    
    if (!testZ80State("INC B half carry", testOutput, z80, 1, 0x20, 0x20, 3, 4, 5, 6, 7, 3, 0) ||
        !testInt("INC B run result", testOutput, run, 1))
    {
        return 0;
    }
    
    return 1;
}

int testDEC_B(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;

    z80->a = 1; z80->f = 0xF0; z80->b = 2; z80->c = 3;
    z80->d = 4; z80->e = 5; z80->h = 6; z80->l = 7;
    z80->pc = 0; z80->sp = 0;

    memory[0] = Z80_DEC_B;
    memory[1] = Z80_DEC_B;
    memory[2] = Z80_DEC_B;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("DEC B", testOutput, z80, 1, 0x70, 1, 3, 4, 5, 6, 7, 1, 0) ||
        !testInt("DEC B run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting zero flag
    z80->b = 0x1;
    z80->f = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    
    if (!testZ80State("DEC B zero", testOutput, z80, 1, 0xC0, 0, 3, 4, 5, 6, 7, 2, 0) ||
        !testInt("DEC B zero run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting half flag
    z80->b = 0x10;
    z80->f = 0;
    run = runZ80CPU(z80, memoryMap, 1);
    
    if (!testZ80State("DEC B half carry", testOutput, z80, 1, 0x40, 0xF, 3, 4, 5, 6, 7, 3, 0) ||
        !testInt("DEC B half carry run result", testOutput, run, 1))
    {
        return 0;
    }
    
    return 1;
}

int testLD_B_d8(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;

    z80->a = 1; z80->f = 0xF0; z80->b = 2; z80->c = 3;
    z80->d = 4; z80->e = 5; z80->h = 6; z80->l = 7;
    z80->pc = 0; z80->sp = 0;

    memory[0] = Z80_LD_B_d8;
    memory[1] = 23;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("LD B d8", testOutput, z80, 1, 0xF0, 23, 3, 4, 5, 6, 7, 2, 0) ||
        !testInt("LD B d8 run result", testOutput, run, 2))
    {
        return 0;
    }
    
    return 1;
}

int testRLCA(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;

    z80->a = 1; z80->f = 0xF0; z80->b = 2; z80->c = 3;
    z80->d = 4; z80->e = 5; z80->h = 6; z80->l = 7;
    z80->pc = 0; z80->sp = 0;

    memory[0] = Z80_RLCA;
    memory[1] = Z80_RLCA;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("RLCA", testOutput, z80, 2, 0x00, 2, 3, 4, 5, 6, 7, 1, 0) ||
        !testInt("RLCA run result", testOutput, run, 1))
    {
        return 0;
    }

    z80->a = 0x80;
    run = runZ80CPU(z80, memoryMap, 1);
    
    if (!testZ80State("RLCA set carry", testOutput, z80, 1, 0x10, 2, 3, 4, 5, 6, 7, 2, 0) ||
        !testInt("RLCA run result", testOutput, run, 1))
    {
        return 0;
    }

    return 1;
}

int testLD_A16_SP(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;

    z80->a = 1; z80->f = 0xF0; z80->b = 2; z80->c = 3;
    z80->d = 4; z80->e = 5; z80->h = 6; z80->l = 7;
    z80->pc = 0; z80->sp = 0;

    memory[0] = Z80_LD_a16_SP;
    memory[1] = 0xE8;
    memory[2] = 0x3C;

    run = runZ80CPU(z80, memoryMap, 1);

    if (!testZ80State("LD a16 SP", testOutput, z80, 1, 0xF0, 2, 3, 4, 5, 6, 7, 3, 0xE83C) ||
        !testInt("LD a16 SP run result", testOutput, run, 5))
    {
        return 0;
    }
    
    return 1;
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