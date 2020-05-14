#include "z80_test.h"
#include "../memory_map.h"

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
        actual->pc != expected->pc || actual->sp != expected->sp || actual->stopReason != expected->stopReason)
    {
        sprintf(testOutput, 
            "Failed z80: %s\n"
            "   A  F  B  C  D  E  H  L  PC   SP\n"
            " E %02X %02X %02X %02X %02X %02X %02X %02X %04X %04X\n"
            " A %02X %02X %02X %02X %02X %02X %02X %02X %04X %04X\n"
            " ASR %d ESR %d",
            testName, 
            expected->a, expected->f, expected->b, expected->c, 
            expected->d, expected->e, expected->h, expected->l, 
            expected->pc, expected->sp,
            actual->a, actual->f, actual->b, actual->c, 
            actual->d, actual->e, actual->h, actual->l, 
            actual->pc, actual->sp,
            actual->stopReason, expected->stopReason
        );

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
        !run0x0Tests(&z80, memoryMap, memory, subTestOutput) ||
        !run0x1Tests(&z80, memoryMap, memory, subTestOutput) ||
        0)
    {
        sprintf(testOutput, "runZ80CPU 0x%X\n%s", &runZ80CPU, subTestOutput);
        return 0;
    }

	sprintf(testOutput, "Tests Passed");

    return 1;
}