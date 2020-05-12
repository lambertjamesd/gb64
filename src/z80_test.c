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

int testZ80STate(
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

    if (!testZ80STate("NOP", testOutput, z80, 1, 0xF0, 2, 3, 4, 5, 6, 7, 1, 0) ||
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

    if (!testZ80STate("LD BC d16", testOutput, z80, 1, 0xF0, 13, 17, 4, 5, 6, 7, 3, 0) ||
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
        !testZ80STate("LD BC A", testOutput, z80, 63, 0xF0, 0x81, 5, 4, 5, 6, 7, 1, 0) ||
        !testInt("LD BC A run result", testOutput, run, 2) ||
        !testInt("LD BC A stored value", testOutput, memory[0x15], 63))
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

    for (i = 0; i < MEMORY_MAP_SIZE; ++i)
    {
        memoryMap[i] = memory;
    }

    zeroMemory(memory, 512);
    initializeZ80(&z80);

    if (
        // !testNOP(&z80, memoryMap, memory, testOutput) ||
        // !testLD_BC_d16(&z80, memoryMap, memory, testOutput) ||
        !testLD_BC_A(&z80, memoryMap, memory, testOutput) ||
        0)
    {
        return 0;
    }

	sprintf(testOutput, "Tests Passed");

    return 1;
}