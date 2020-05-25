
#include "cpu_test.h"

int testRET_NC(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = GB_FLAGS_C;
    cpu->sp = 0x8020;
    expected = *cpu;
    expected.pc = 1;

    memory->internalRam[0] = CPU_RET_NC;
    memory->internalRam[1] = CPU_RET_NC;

    memory->internalRam[0x20] = 0xE0;
    memory->internalRam[0x21] = 0x34;

    run = runCPUCPU(cpu, memory, 1);

    if (!testCPUState("RET NC", testOutput, cpu, &expected) ||
        !testInt("RET NC run result", testOutput, run, 2))
    {
        return 0;
    }
    
    cpu->f = 0;
    expected.f = 0;
    expected.pc = 0x34E0;
    expected.sp = 0x8022;
    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("RET NC branch", testOutput, cpu, &expected) &&
        testInt("RET NC branch run result", testOutput, run, 3) &&
        1
    ;
}

int testPOP_DE(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->sp = 0x8020;
    expected = *cpu;
    expected.pc = 1;
    expected.sp = 0x8022;
    expected.d = 0x34;
    expected.e = 0xE0;

    memory->internalRam[0] = CPU_POP_DE;

    memory->internalRam[0x20] = 0xE0;
    memory->internalRam[0x21] = 0x34;

    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("POP DE branch", testOutput, cpu, &expected) &&
        testInt("POP DE branch run result", testOutput, run, 3) &&
        1
    ;
}

int testJP_NC(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = GB_FLAGS_C;
    expected = *cpu;
    expected.pc = 3;

    memory->internalRam[0] = CPU_JP_NC_a16;
    memory->internalRam[1] = 0x20;
    memory->internalRam[2] = 0x00;
    memory->internalRam[3] = CPU_JP_NC_a16;
    memory->internalRam[4] = 0x30;
    memory->internalRam[5] = 0x00;

    run = runCPUCPU(cpu, memory, 1);

    if (!testCPUState("JP NC", testOutput, cpu, &expected) ||
        !testInt("JP NC run result", testOutput, run, 3))
    {
        return 0;
    }
    
    cpu->f = 0;
    expected.f = 0;
    expected.pc = 0x0030;
    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("JP NC branch", testOutput, cpu, &expected) &&
        testInt("JP NC branch run result", testOutput, run, 3) &&
        1
    ;
}

int testCALL_NC(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = GB_FLAGS_C;
    cpu->sp = 0x8022;
    expected = *cpu;
    expected.pc = 3;

    memory->internalRam[0] = CPU_CALL_NC;
    memory->internalRam[1] = 0x20;
    memory->internalRam[2] = 0x00;
    memory->internalRam[3] = CPU_CALL_NC;
    memory->internalRam[4] = 0x30;
    memory->internalRam[5] = 0x00;

    run = runCPUCPU(cpu, memory, 1);

    if (!testCPUState("CALL NC", testOutput, cpu, &expected) ||
        !testInt("CALL NC run result", testOutput, run, 3))
    {
        return 0;
    }
    
    cpu->f = 0;
    expected.f = 0;
    expected.pc = 0x0030;
    expected.sp = 0x8020;
    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("CALL NC branch", testOutput, cpu, &expected) &&
        testInt("CALL NC branch run result", testOutput, run, 5) &&
        testInt("CALL NC saved return", testOutput, memory->internalRam[0x20], 0x6) &&
        testInt("CALL NC saved return", testOutput, memory->internalRam[0x21], 0x0) &&
        1
    ;
}

int testPUSH_DE(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = 0;
    cpu->sp = 0x8022;
    cpu->d = 0x45;
    cpu->e = 0x24;
    expected = *cpu;
    expected.pc = 1;
    expected.sp = 0x8020;

    memory->internalRam[0] = CPU_PUSH_DE;

    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("PUSH DE", testOutput, cpu, &expected) &&
        testInt("PUSH DE run result", testOutput, run, 4) &&
        testInt("PUSH DE pushed value", testOutput, memory->internalRam[0x20], 0x24) &&
        testInt("PUSH DE pushed value", testOutput, memory->internalRam[0x21], 0x45) &&
        1
    ;
}


int testRET_C(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = 0;
    cpu->sp = 0x8020;
    expected = *cpu;
    expected.pc = 1;

    memory->internalRam[0] = CPU_RET_C;
    memory->internalRam[1] = CPU_RET_C;

    memory->internalRam[0x20] = 0xE0;
    memory->internalRam[0x21] = 0x34;

    run = runCPUCPU(cpu, memory, 1);

    if (!testCPUState("RET C", testOutput, cpu, &expected) ||
        !testInt("RET C run result", testOutput, run, 2))
    {
        return 0;
    }
    
    cpu->f = GB_FLAGS_C;
    expected.f = GB_FLAGS_C;
    expected.pc = 0x34E0;
    expected.sp = 0x8022;
    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("RET C branch", testOutput, cpu, &expected) &&
        testInt("RET C branch run result", testOutput, run, 3) &&
        1
    ;
}

int testRETI(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->sp = 0x8020;
    expected = *cpu;
    expected.pc = 0x34E0;
    expected.sp = 0x8022;
    expected.interrupts = GB_INTERRUPTS_ENABLED;

    memory->internalRam[0] = CPU_RETI;

    memory->internalRam[0x20] = 0xE0;
    memory->internalRam[0x21] = 0x34;

    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("RETI", testOutput, cpu, &expected) &&
        testInt("RETI run result", testOutput, run, 3) &&
        1
    ;
}

int testJP_C(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = 0;
    expected = *cpu;
    expected.pc = 3;

    memory->internalRam[0] = CPU_JP_C_a16;
    memory->internalRam[1] = 0x20;
    memory->internalRam[2] = 0x00;
    memory->internalRam[3] = CPU_JP_C_a16;
    memory->internalRam[4] = 0x30;
    memory->internalRam[5] = 0x00;

    run = runCPUCPU(cpu, memory, 1);

    if (!testCPUState("JP C", testOutput, cpu, &expected) ||
        !testInt("JP C run result", testOutput, run, 3))
    {
        return 0;
    }
    
    cpu->f = GB_FLAGS_C;
    expected.f = GB_FLAGS_C;
    expected.pc = 0x0030;
    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("JP C branch", testOutput, cpu, &expected) &&
        testInt("JP C branch run result", testOutput, run, 3) &&
        1
    ;
}

int testCALL_C(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = 0;
    cpu->sp = 0x8022;
    expected = *cpu;
    expected.pc = 3;

    memory->internalRam[0] = CPU_CALL_C;
    memory->internalRam[1] = 0x20;
    memory->internalRam[2] = 0x00;
    memory->internalRam[3] = CPU_CALL_C;
    memory->internalRam[4] = 0x30;
    memory->internalRam[5] = 0x00;

    run = runCPUCPU(cpu, memory, 1);

    if (!testCPUState("CALL C", testOutput, cpu, &expected) ||
        !testInt("CALL C run result", testOutput, run, 3))
    {
        return 0;
    }
    
    cpu->f = GB_FLAGS_C;
    expected.f = GB_FLAGS_C;
    expected.pc = 0x0030;
    expected.sp = 0x8020;
    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("CALL C branch", testOutput, cpu, &expected) &&
        testInt("CALL C branch run result", testOutput, run, 5) &&
        testInt("CALL C saved return", testOutput, memory->internalRam[0x20], 0x6) &&
        testInt("CALL C saved return", testOutput, memory->internalRam[0x21], 0x0) &&
        1
    ;
}

int run0xDTests(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    return 
        testRET_NC(cpu, memory, testOutput) &&
        testPOP_DE(cpu, memory, testOutput) &&
        testJP_NC(cpu, memory, testOutput) &&
        testCALL_NC(cpu, memory, testOutput) &&
        testPUSH_DE(cpu, memory, testOutput) &&
        testSingleADD(cpu, memory, testOutput, d8_REGISTER_INDEX, CPU_SUB_A_d8, 0) &&
        testSingleADD(cpu, memory, testOutput, d8_REGISTER_INDEX, CPU_SUB_A_d8, 1) &&
        testRST(cpu, memory, testOutput, CPU_RST_10H, 0x10) &&
        testRET_C(cpu, memory, testOutput) &&
        testRETI(cpu, memory, testOutput) &&
        testJP_C(cpu, memory, testOutput) &&
        testCALL_C(cpu, memory, testOutput) &&
        testSingleADD(cpu, memory, testOutput, d8_REGISTER_INDEX, CPU_SBC_A_d8, 0) &&
        testSingleADD(cpu, memory, testOutput, d8_REGISTER_INDEX, CPU_SBC_A_d8, 1) &&
        testRST(cpu, memory, testOutput, CPU_RST_18H, 0x18) &&
        1
    ;
}