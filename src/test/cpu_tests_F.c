
#include "cpu_test.h"

int testLDH_A_a8(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 2;
    expected.a = 0x3E;

    memory->internalRam[0] = CPU_LDH_A_a8;
    memory->internalRam[1] = 0xF0;
    memory->miscBytes[0x1F0] = 0x3E;

    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("LDH A a8 branch", testOutput, cpu, &expected) &&
        testInt("LDH A a8 branch run result", testOutput, run, 3) &&
        1
    ;
}

int testPOP_AF(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->sp = 0x8020;
    expected = *cpu;
    expected.pc = 1;
    expected.sp = 0x8022;
    expected.a = 0x34;
    expected.f = 0xE0;

    memory->internalRam[0] = CPU_POP_AF;

    memory->internalRam[0x20] = 0xE2;
    memory->internalRam[0x21] = 0x34;

    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("POP AF branch", testOutput, cpu, &expected) &&
        testInt("POP AF branch run result", testOutput, run, 3) &&
        1
    ;
}

int testLDH_A_C_ADDR(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->a = 0x3E;
    cpu->c = 0xF2;
    expected = *cpu;
    expected.pc = 1;
    expected.a = 0x24;

    memory->internalRam[0] = CPU_LD_A_C_ADDR;
    memory->miscBytes[0x1F2] = 0x24;

    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("LD A (C) A branch", testOutput, cpu, &expected) &&
        testInt("LD A (C) A branch run result", testOutput, run, 2) &&
        1
    ;
}

int testDI(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->sp = 0x8022;
    cpu->a = 0x45;
    cpu->f = 0x24;
    cpu->interrupts = GB_INTERRUPTS_ENABLED;
    expected = *cpu;
    expected.pc = 1;
    expected.interrupts = 0;

    memory->internalRam[0] = CPU_DI;

    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("DI", testOutput, cpu, &expected) &&
        testInt("DI run result", testOutput, run, 1) &&
        1
    ;
}

int testPUSH_AF(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->sp = 0x8022;
    cpu->a = 0x45;
    cpu->f = 0x24;
    expected = *cpu;
    expected.pc = 1;
    expected.sp = 0x8020;

    memory->internalRam[0] = CPU_PUSH_AF;

    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("PUSH HL", testOutput, cpu, &expected) &&
        testInt("PUSH HL run result", testOutput, run, 4) &&
        testInt("PUSH HL pushed value", testOutput, memory->internalRam[0x20], 0x24) &&
        testInt("PUSH HL pushed value", testOutput, memory->internalRam[0x21], 0x45) &&
        1
    ;
}

int testLD_HL_SP_r8(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = 0;
    cpu->sp = 0x8020;
    cpu->h = 0x45;
    cpu->l = 0x24;
    expected = *cpu;
    expected.pc = 2;
    expected.sp = 0x8020;
    expected.h = 0x80;
    expected.l = 0x40;

    memory->internalRam[0] = CPU_LD_HL_SP_r8;
    memory->internalRam[1] = 0x20;
    memory->internalRam[2] = CPU_LD_HL_SP_r8;
    memory->internalRam[3] = 0xF0;

    run = runCPUCPU(cpu, memory, 1);

    if (
        !testCPUState("LD HL SP r8", testOutput, cpu, &expected) ||
        !testInt("LD HL SP r8 run result", testOutput, run, 3)
    )
    {
        return 0;
    }

    run = runCPUCPU(cpu, memory, 1);
    expected.pc = 4;
    expected.h = 0x80;
    expected.l = 0x10;

    return testCPUState("LD HL SP r8", testOutput, cpu, &expected) &&
        testInt("LD HL SP r8 run result", testOutput, run, 3) &&
        1
    ;
}

int testLD_SP_HL(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->h = 0x45;
    cpu->l = 0x24;
    expected = *cpu;
    expected.sp = 0x4524;
    expected.pc = 1;

    memory->internalRam[0] = CPU_SP_HL;

    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("LD SP HL", testOutput, cpu, &expected) &&
        testInt("LD SP HL run result", testOutput, run, 2) &&
        1
    ;
}

int testLD_A_a16(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.a = 0x72;
    expected.pc = 3;

    memory->internalRam[0] = CPU_LD_A_a16;
    memory->internalRam[1] = 0x40;
    memory->internalRam[2] = 0x80;
    memory->internalRam[0x40] = 0x72;

    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("LD a16 A", testOutput, cpu, &expected) &&
        testInt("LD a16 A run result", testOutput, run, 4) &&
        1
    ;
}

int testEI(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 1;
    expected.interrupts = GB_INTERRUPTS_ENABLED;

    memory->internalRam[0] = CPU_EI;

    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("EI HL", testOutput, cpu, &expected) &&
        testInt("EI run result", testOutput, run, 1) &&
        1
    ;
}

int run0xFTests(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    return 
        testLDH_A_a8(cpu, memory, testOutput) &&
        testPOP_AF(cpu, memory, testOutput) &&
        testLDH_A_C_ADDR(cpu, memory, testOutput) &&
        testDI(cpu, memory, testOutput) &&
        testPUSH_AF(cpu, memory, testOutput) &&
        testSingleBitwise(cpu, memory, testOutput, d8_REGISTER_INDEX, CPU_OR_A_d8) &&
        testRST(cpu, memory, testOutput, CPU_RST_30H, 0x30) &&
        testLD_HL_SP_r8(cpu, memory, testOutput) && 
        testLD_SP_HL(cpu, memory, testOutput) && 
        testLD_A_a16(cpu, memory, testOutput) &&
        testEI(cpu, memory, testOutput) &&
        testSingleADD(cpu, memory, testOutput, d8_REGISTER_INDEX, CPU_CP_A_d8, 1) &&
        testRST(cpu, memory, testOutput, CPU_RST_38H, 0x38) &&
        1
    ;
}