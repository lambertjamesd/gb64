
#include "cpu_test.h"

int testLDH_a8_A(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->a = 0x3E;
    expected = *cpu;
    expected.pc = 2;

    memory->internalRam[0] = CPU_LDH_a8_A;
    memory->internalRam[1] = 0xF0;

    run = runCPU(cpu, memory, 1);

    return testCPUState("LDH a8 A branch", testOutput, cpu, &expected) &&
        testInt("LDH a8 A branch run result", testOutput, run, 3) &&
        testInt("LDH a8 A store result", testOutput, memory->miscBytes[0x1F0], 0x3E) &&
        1
    ;
}

int testPOP_HL(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->sp = 0x8020;
    expected = *cpu;
    expected.pc = 1;
    expected.sp = 0x8022;
    expected.h = 0x34;
    expected.l = 0xE0;

    memory->internalRam[0] = CPU_POP_HL;

    memory->internalRam[0x20] = 0xE0;
    memory->internalRam[0x21] = 0x34;

    run = runCPU(cpu, memory, 1);

    return testCPUState("POP HL branch", testOutput, cpu, &expected) &&
        testInt("POP HL branch run result", testOutput, run, 3) &&
        1
    ;
}

int testLDH_C_ADDR_A(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->a = 0x3E;
    cpu->c = 0xF0;
    expected = *cpu;
    expected.pc = 1;

    memory->internalRam[0] = CPU_LD_C_ADDR_A;

    run = runCPU(cpu, memory, 1);

    return testCPUState("LD (C) A branch", testOutput, cpu, &expected) &&
        testInt("LD (C) A branch run result", testOutput, run, 2) &&
        testInt("LD (C) A store result", testOutput, memory->miscBytes[0x1F0], 0x3E) &&
        1
    ;
}

int testPUSH_HL(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = 0;
    cpu->sp = 0x8022;
    cpu->h = 0x45;
    cpu->l = 0x24;
    expected = *cpu;
    expected.pc = 1;
    expected.sp = 0x8020;

    memory->internalRam[0] = CPU_PUSH_HL;

    run = runCPU(cpu, memory, 1);

    return testCPUState("PUSH HL", testOutput, cpu, &expected) &&
        testInt("PUSH HL run result", testOutput, run, 4) &&
        testInt("PUSH HL pushed value", testOutput, memory->internalRam[0x20], 0x24) &&
        testInt("PUSH HL pushed value", testOutput, memory->internalRam[0x21], 0x45) &&
        1
    ;
}

int testADD_SP_r8(struct CPUState* cpu, struct Memory* memory, char* testOutput)
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
    expected.sp = 0x8040;

    memory->internalRam[0] = CPU_ADD_SP_r8;
    memory->internalRam[1] = 0x20;
    memory->internalRam[2] = CPU_ADD_SP_r8;
    memory->internalRam[3] = -0x30;

    run = runCPU(cpu, memory, 1);

    if (
        !testCPUState("ADD SP r8 HL", testOutput, cpu, &expected) ||
        !testInt("ADD SP r8 run result", testOutput, run, 4)
    )
    {
        return 0;
    }

    run = runCPU(cpu, memory, 1);
    expected.pc = 4;
    expected.sp = 0x8010;
    expected.f = GB_FLAGS_C;

    return testCPUState("ADD SP r8 HL", testOutput, cpu, &expected) &&
        testInt("ADD SP r8 run result", testOutput, run, 4) &&
        1
    ;
}

int testJP_HL(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->h = 0x45;
    cpu->l = 0x24;
    expected = *cpu;
    expected.pc = 0x4524;

    memory->internalRam[0] = CPU_JP_HL;

    run = runCPU(cpu, memory, 1);

    return testCPUState("JP HL", testOutput, cpu, &expected) &&
        testInt("JP HL run result", testOutput, run, 1) &&
        1
    ;
}

int testLD_a16_A(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->a = 0x72;
    expected = *cpu;
    expected.pc = 3;

    memory->internalRam[0] = CPU_LD_a16_A;
    memory->internalRam[1] = 0x40;
    memory->internalRam[2] = 0x80;
    memory->internalRam[0x40] = 0x72;

    run = runCPU(cpu, memory, 1);

    return testCPUState("LD a16 A branch", testOutput, cpu, &expected) &&
        testInt("LD a16 A branch run result", testOutput, run, 4) &&
        1
    ;
}

int run0xETests(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    return 
        testLDH_a8_A(cpu, memory, testOutput) &&
        testPOP_HL(cpu, memory, testOutput) &&
        testLDH_C_ADDR_A(cpu, memory, testOutput) &&
        testPUSH_HL(cpu, memory, testOutput) &&
        testSingleBitwise( cpu, memory, testOutput, d8_REGISTER_INDEX, CPU_AND_A_d8) &&
        testRST(cpu, memory, testOutput, CPU_RST_20H, 0x20) &&
        testADD_SP_r8(cpu, memory, testOutput) && 
        testJP_HL(cpu, memory, testOutput) && 
        testLD_a16_A(cpu, memory, testOutput) &&
        testSingleBitwise( cpu, memory, testOutput, d8_REGISTER_INDEX, CPU_XOR_A_d8) &&
        testRST(cpu, memory, testOutput, CPU_RST_28H, 0x28) &&
        1
    ;
}