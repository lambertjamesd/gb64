
#include "cpu_test.h"

int testRET_NZ(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = GB_FLAGS_Z;
    cpu->sp = 0x8020;
    expected = *cpu;
    expected.pc = 1;

    memory->internalRam[0] = CPU_RET_NZ;
    memory->internalRam[1] = CPU_RET_NZ;

    memory->internalRam[0x20] = 0xE0;
    memory->internalRam[0x21] = 0x34;

    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("RET NZ", testOutput, cpu, &expected) ||
        !testInt("RET NZ run result", testOutput, run, 2))
    {
        return 0;
    }
    
    cpu->f = 0;
    expected.f = 0;
    expected.pc = 0x34E0;
    expected.sp = 0x8022;
    run = runCPU(cpu, memory, 1, 0);

    return testCPUState("RET NZ branch", testOutput, cpu, &expected) &&
        testInt("RET NZ branch run result", testOutput, run, 3) &&
        1
    ;
}

int testPOP_BC(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->sp = 0x8020;
    expected = *cpu;
    expected.pc = 1;
    expected.sp = 0x8022;
    expected.b = 0x34;
    expected.c = 0xE0;

    memory->internalRam[0] = CPU_POP_BC;

    memory->internalRam[0x20] = 0xE0;
    memory->internalRam[0x21] = 0x34;

    run = runCPU(cpu, memory, 1, 0);

    return testCPUState("POP BC branch", testOutput, cpu, &expected) &&
        testInt("POP BC branch run result", testOutput, run, 3) &&
        1
    ;
}

int testJP_NZ(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = GB_FLAGS_Z;
    expected = *cpu;
    expected.pc = 3;

    memory->internalRam[0] = CPU_JP_NZ_a16;
    memory->internalRam[1] = 0x20;
    memory->internalRam[2] = 0x00;
    memory->internalRam[3] = CPU_JP_NZ_a16;
    memory->internalRam[4] = 0x30;
    memory->internalRam[5] = 0x00;

    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("JP NZ", testOutput, cpu, &expected) ||
        !testInt("JP NZ run result", testOutput, run, 3))
    {
        return 0;
    }
    
    cpu->f = 0;
    expected.f = 0;
    expected.pc = 0x0030;
    run = runCPU(cpu, memory, 1, 0);

    return testCPUState("JP NZ branch", testOutput, cpu, &expected) &&
        testInt("JP NZ branch run result", testOutput, run, 3) &&
        1
    ;
}

int testJP(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = 0;
    expected = *cpu;
    expected.pc = 0x0020;

    memory->internalRam[0] = CPU_JP_a16;
    memory->internalRam[1] = 0x20;
    memory->internalRam[2] = 0x00;

    run = runCPU(cpu, memory, 1, 0);

    return testCPUState("JP", testOutput, cpu, &expected) &&
        testInt("JP run result", testOutput, run, 3) &&
        1
    ;
}

int testCALL_NZ(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = GB_FLAGS_Z;
    cpu->sp = 0x8022;
    expected = *cpu;
    expected.pc = 3;

    memory->internalRam[0] = CPU_CALL_NZ;
    memory->internalRam[1] = 0x20;
    memory->internalRam[2] = 0x00;
    memory->internalRam[3] = CPU_CALL_NZ;
    memory->internalRam[4] = 0x30;
    memory->internalRam[5] = 0x00;

    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("CALL NZ", testOutput, cpu, &expected) ||
        !testInt("CALL NZ run result", testOutput, run, 3))
    {
        return 0;
    }
    
    cpu->f = 0;
    expected.f = 0;
    expected.pc = 0x0030;
    expected.sp = 0x8020;
    run = runCPU(cpu, memory, 1, 0);

    return testCPUState("CALL NZ branch", testOutput, cpu, &expected) &&
        testInt("CALL NZ branch run result", testOutput, run, 5) &&
        testInt("CALL NZ saved return", testOutput, memory->internalRam[0x20], 0x6) &&
        testInt("CALL NZ saved return", testOutput, memory->internalRam[0x21], 0x0) &&
        1
    ;
}

int testPUSH_BC(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = 0;
    cpu->sp = 0x8022;
    cpu->b = 0x45;
    cpu->c = 0x24;
    expected = *cpu;
    expected.pc = 1;
    expected.sp = 0x8020;

    memory->internalRam[0] = CPU_PUSH_BC;

    run = runCPU(cpu, memory, 1, 0);

    return testCPUState("PUSH BC", testOutput, cpu, &expected) &&
        testInt("PUSH BC run result", testOutput, run, 4) &&
        testInt("PUSH BC pushed value", testOutput, memory->internalRam[0x20], 0x24) &&
        testInt("PUSH BC pushed value", testOutput, memory->internalRam[0x21], 0x45) &&
        1
    ;
}

int testRST(struct CPUState* cpu, struct Memory* memory, char* testOutput, int instruction, int targetAddress)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = 0;
    cpu->sp = 0x8032;
    cpu->pc = 0x2010;
    expected = *cpu;
    expected.pc = targetAddress;
    expected.sp = 0x8030;

    memory->internalRam[0x10] = instruction;

    run = runCPU(cpu, memory, 1, 0);

    return testCPUState("RST", testOutput, cpu, &expected) &&
        testInt("RST run result", testOutput, run, 3) &&
        testInt("RST saved return", testOutput, memory->internalRam[0x30], 0x11) &&
        testInt("RST saved return", testOutput, memory->internalRam[0x31], 0x20) &&
        1
    ;
}

int testRET_Z(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = 0;
    cpu->sp = 0x8020;
    expected = *cpu;
    expected.pc = 1;

    memory->internalRam[0] = CPU_RET_Z;
    memory->internalRam[1] = CPU_RET_Z;

    memory->internalRam[0x20] = 0xE0;
    memory->internalRam[0x21] = 0x34;

    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("RET Z", testOutput, cpu, &expected) ||
        !testInt("RET Z run result", testOutput, run, 2))
    {
        return 0;
    }
    
    cpu->f = GB_FLAGS_Z;
    expected.f = GB_FLAGS_Z;
    expected.pc = 0x34E0;
    expected.sp = 0x8022;
    run = runCPU(cpu, memory, 1, 0);

    return testCPUState("RET Z branch", testOutput, cpu, &expected) &&
        testInt("RET Z branch run result", testOutput, run, 3) &&
        1
    ;
}

int testRET(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->sp = 0x8020;
    expected = *cpu;
    expected.pc = 0x34E0;
    expected.sp = 0x8022;

    memory->internalRam[0] = CPU_RET;
    memory->internalRam[1] = CPU_RET;

    memory->internalRam[0x20] = 0xE0;
    memory->internalRam[0x21] = 0x34;

    run = runCPU(cpu, memory, 1, 0);

    return testCPUState("RET branch", testOutput, cpu, &expected) &&
        testInt("RET branch run result", testOutput, run, 3) &&
        1
    ;
}

int testJP_Z(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = 0;
    expected = *cpu;
    expected.pc = 3;

    memory->internalRam[0] = CPU_JP_Z_a16;
    memory->internalRam[1] = 0x20;
    memory->internalRam[2] = 0x00;
    memory->internalRam[3] = CPU_JP_Z_a16;
    memory->internalRam[4] = 0x30;
    memory->internalRam[5] = 0x00;

    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("JP Z", testOutput, cpu, &expected) ||
        !testInt("JP Z run result", testOutput, run, 3))
    {
        return 0;
    }
    
    cpu->f = GB_FLAGS_Z;
    expected.f = GB_FLAGS_Z;
    expected.pc = 0x0030;
    run = runCPU(cpu, memory, 1, 0);

    return testCPUState("JP Z branch", testOutput, cpu, &expected) &&
        testInt("JP Z branch run result", testOutput, run, 3) &&
        1
    ;
}

int testCB(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    return 1;
}

int testCALL_Z(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = 0;
    cpu->sp = 0x8022;
    expected = *cpu;
    expected.pc = 3;

    memory->internalRam[0] = CPU_CALL_Z;
    memory->internalRam[1] = 0x20;
    memory->internalRam[2] = 0x00;
    memory->internalRam[3] = CPU_CALL_Z;
    memory->internalRam[4] = 0x30;
    memory->internalRam[5] = 0x00;

    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("CALL Z", testOutput, cpu, &expected) ||
        !testInt("CALL Z run result", testOutput, run, 3))
    {
        return 0;
    }
    
    cpu->f = GB_FLAGS_Z;
    expected.f = GB_FLAGS_Z;
    expected.pc = 0x0030;
    expected.sp = 0x8020;
    run = runCPU(cpu, memory, 1, 0);

    return testCPUState("CALL Z branch", testOutput, cpu, &expected) &&
        testInt("CALL Z branch run result", testOutput, run, 5) &&
        testInt("CALL Z saved return", testOutput, memory->internalRam[0x20], 0x6) &&
        testInt("CALL Z saved return", testOutput, memory->internalRam[0x21], 0x0) &&
        1
    ;
}

int testCALL(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = 0;
    cpu->sp = 0x8022;
    expected = *cpu;
    expected.pc = 0x0030;
    expected.sp = 0x8020;

    memory->internalRam[0] = CPU_CALL;
    memory->internalRam[1] = 0x30;
    memory->internalRam[2] = 0x00;

    run = runCPU(cpu, memory, 1, 0);

    return testCPUState("CALL", testOutput, cpu, &expected) &&
        testInt("CALL run result", testOutput, run, 5) &&
        testInt("CALL saved return", testOutput, memory->internalRam[0x20], 0x3) &&
        testInt("CALL saved return", testOutput, memory->internalRam[0x21], 0x0) &&
        1
    ;
}

int run0xCTests(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    return 
        testRET_NZ(cpu, memory, testOutput) &&
        testPOP_BC(cpu, memory, testOutput) &&
        testJP_NZ(cpu, memory, testOutput) &&
        testJP(cpu, memory, testOutput) &&
        testCALL_NZ(cpu, memory, testOutput) &&
        testPUSH_BC(cpu, memory, testOutput) &&
        testSingleADD(cpu, memory, testOutput, d8_REGISTER_INDEX, CPU_ADD_A_d8, 0) &&
        testSingleADD(cpu, memory, testOutput, d8_REGISTER_INDEX, CPU_ADD_A_d8, 1) &&
        testRST(cpu, memory, testOutput, CPU_RST_00H, 0x00) &&
        testRET_Z(cpu, memory, testOutput) &&
        testRET(cpu, memory, testOutput) &&
        testJP_Z(cpu, memory, testOutput) &&
        runPrefixCBTests(cpu, memory, testOutput) &&
        testCB(cpu, memory, testOutput) &&
        testCALL_Z(cpu, memory, testOutput) &&
        testCALL(cpu, memory, testOutput) &&
        testSingleADD(cpu, memory, testOutput, d8_REGISTER_INDEX, CPU_ADC_A_d8, 0) &&
        testSingleADD(cpu, memory, testOutput, d8_REGISTER_INDEX, CPU_ADC_A_d8, 1) &&
        testRST(cpu, memory, testOutput, CPU_RST_08H, 0x08) &&
        1
    ;
}