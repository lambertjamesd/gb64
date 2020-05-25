#include "cpu_test.h"

int testNOP(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 1;

    memory->internalRam[0] = CPU_NOP;

    run = runCPU(cpu, memory, 1);

    return 
        testCPUState("NOP", testOutput, cpu, &expected) &&
        testInt("NOP run result", testOutput, run, 1) &&
        1
    ;
}

int testLD_BC_d16(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 3;
    expected.b = 13;
    expected.c = 17;

    memory->internalRam[0] = CPU_LD_BC_d16;
    memory->internalRam[1] = 17;
    memory->internalRam[2] = 13;

    run = runCPU(cpu, memory, 1);

    return 
        testCPUState("LD BC d16", testOutput, cpu, &expected) &&
        testInt("LD BC d16 run result", testOutput, run, 3) &&
        1
    ;
}

int testLD_BC_A(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);

    cpu->b = 0x81;
    cpu->c = 0x05;
    cpu->a = 63;

    expected = *cpu;
    expected.pc = 1;

    memory->internalRam[0] = CPU_LD_BC_A;

    run = runCPU(cpu, memory, 1);

    return 
        testCPUState("LD BC A", testOutput, cpu, &expected) &&
        testInt("LD BC A run result", testOutput, run, 2) &&
        testInt("LD BC A stored value", testOutput, memory->internalRam[0x105], 63) &&
        1
    ;
}

int testINC_BC(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);

    cpu->b = 1;
    cpu->c = 3;

    expected = *cpu;
    expected.pc = 1;
    expected.b = 1;
    expected.c = 4;

    memory->internalRam[0] = CPU_INC_BC;
    memory->internalRam[1] = CPU_INC_BC;

    run = runCPU(cpu, memory, 1);

    if (!testCPUState("INC BC", testOutput, cpu, &expected) ||
        !testInt("INC BC run result", testOutput, run, 2))
    {
        return 0;
    }

    cpu->b = 0; cpu->c = 0xFF;
    expected.b = 1;
    expected.c = 0;
    expected.pc = 2;
    
    run = runCPU(cpu, memory, 1);

    return testCPUState("INC BC rollover", testOutput, cpu, &expected);
}

int testINC_B(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 1;
    expected.b = 1;

    memory->internalRam[0] = CPU_INC_B;
    memory->internalRam[1] = CPU_INC_B;
    memory->internalRam[2] = CPU_INC_B;

    run = runCPU(cpu, memory, 1);

    if (!testCPUState("INC B", testOutput, cpu, &expected) ||
        !testInt("INC B run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting zero flag
    cpu->b = 0xFF;
    cpu->f = 0x0;
    expected.f = 0xA0;
    expected.pc = 2;
    expected.b = 0;
    run = runCPU(cpu, memory, 1);
    
    if (!testCPUState("INC B overflow", testOutput, cpu, &expected) ||
        !testInt("INC B run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting half flag
    cpu->b = 0x1F;
    cpu->f = 0;
    expected.f = 0x20;
    expected.pc = 3;
    expected.b = 0x20;
    run = runCPU(cpu, memory, 1);
    
    return 
        testCPUState("INC B half carry", testOutput, cpu, &expected) &&
        testInt("INC B run result", testOutput, run, 1) &&
        1
    ;
}

int testDEC_B(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.f = 0x40;
    expected.pc = 1;
    expected.b = 0xFF;

    memory->internalRam[0] = CPU_DEC_B;
    memory->internalRam[1] = CPU_DEC_B;
    memory->internalRam[2] = CPU_DEC_B;

    run = runCPU(cpu, memory, 1);

    if (!testCPUState("DEC B", testOutput, cpu, &expected) ||
        !testInt("DEC B run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting zero flag
    cpu->b = 0x1;
    cpu->f = 0;
    run = runCPU(cpu, memory, 1);
    expected.f = 0xC0;
    expected.pc = 2;
    expected.b = 0;
    
    if (!testCPUState("DEC B zero", testOutput, cpu, &expected) ||
        !testInt("DEC B zero run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting half flag
    cpu->b = 0x10;
    cpu->f = 0;
    run = runCPU(cpu, memory, 1);
    expected.f = 0x40;
    expected.pc = 3;
    expected.b = 0x0F;
    
    return 
        testCPUState("DEC B half carry", testOutput, cpu, &expected) &
        testInt("DEC B half carry run result", testOutput, run, 1) &&
        1
    ;
}

int testLD_B_d8(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 2;
    expected.b = 23;

    memory->internalRam[0] = CPU_LD_B_d8;
    memory->internalRam[1] = 23;

    run = runCPU(cpu, memory, 1);

    return 
        testCPUState("LD B d8", testOutput, cpu, &expected) &&
        testInt("LD B d8 run result", testOutput, run, 2) &&
        1
    ;
}

int testRLCA(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 1;
    expected.a = 0x2;
    cpu->a = 0x1;
    cpu->f = 0xF0;

    memory->internalRam[0] = CPU_RLCA;
    memory->internalRam[1] = CPU_RLCA;

    run = runCPU(cpu, memory, 1);

    if (!testCPUState("RLCA", testOutput, cpu, &expected) ||
        !testInt("RLCA run result", testOutput, run, 1))
    {
        return 0;
    }

    cpu->a = 0x80;
    expected.pc = 2;
    expected.a = 0x01;
    expected.f = 0x10;
    run = runCPU(cpu, memory, 1);
    
    return 
        testCPUState("RLCA set carry", testOutput, cpu, &expected) &&
        testInt("RLCA run result", testOutput, run, 1) &&
        1
    ;
}

int testLD_A16_SP(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->sp = 0xE83C;
    expected = *cpu;
    expected.pc = 3;

    memory->internalRam[0] = CPU_LD_a16_SP;
    memory->internalRam[1] = 0x40;
    memory->internalRam[2] = 0x81;

    run = runCPU(cpu, memory, 1);

    return testCPUState("LD a16 SP", testOutput, cpu, &expected) &&
        testInt("LD a16 SP run result", testOutput, run, 5) &&
        testInt("LD a16 SP low bits", testOutput, memory->internalRam[0x140], 0x3C) &&
        testInt("LD a16 SP high bits", testOutput, memory->internalRam[0x141], 0xE8) &&
        1
    ;
}

int testADD_HL_BC(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->h = 0x02;
    cpu->l = 0x80;
    cpu->b = 0x80;
    cpu->c = 0x81;
    expected = *cpu;
    expected.pc = 1;
    expected.h = 0x83;
    expected.l = 0x01;
    expected.f = 0x0;

    memory->internalRam[0] = CPU_ADD_HL_BC;
    memory->internalRam[1] = CPU_ADD_HL_BC;
    memory->internalRam[2] = CPU_ADD_HL_BC;

    run = runCPU(cpu, memory, 1);

    if (!testCPUState("HL BC", testOutput, cpu, &expected) ||
        !testInt("HL BC run result", testOutput, run, 2))
    {
        return 0;
    }
    
    cpu->b = 0xD;
    cpu->c = 0;
    expected.pc = 2;
    expected.h = 0x90;
    expected.l = 0x01;
    expected.b = 0xD;
    expected.c = 0;
    expected.f = 0x20;
    
    run = runCPU(cpu, memory, 1);

    if (!testCPUState("HL BC", testOutput, cpu, &expected) ||
        !testInt("HL BC run result", testOutput, run, 2))
    {
        return 0;
    }

    cpu->b = 0x70;
    run = runCPU(cpu, memory, 1);
    expected.pc = 3;
    expected.h = 0x00;
    expected.l = 0x01;
    expected.b = 0x70;
    expected.f = 0x10;

    return 
        testCPUState("HL BC half", testOutput, cpu, &expected) &&
        testInt("HL BC run result", testOutput, run, 2) &&
        1
    ;
}

int testLD_A_BC(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->b = 0x21;
    cpu->c = 0x30;
    expected = *cpu;
    expected.pc = 1;
    expected.a = 0xA3;

    memory->internalRam[0] = CPU_LD_A_BC;
    memory->internalRam[1] = 23;
    memory->internalRam[0x130] = 0xA3;

    run = runCPU(cpu, memory, 1);

    return 
        testCPUState("LD A BC", testOutput, cpu, &expected) &&
        testInt("LD A BC run result", testOutput, run, 2) &&
        1
    ;
}


int testDEC_BC(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);

    cpu->b = 1;
    cpu->c = 3;

    expected = *cpu;
    expected.pc = 1;
    expected.b = 1;
    expected.c = 2;

    memory->internalRam[0] = CPU_DEC_BC;
    memory->internalRam[1] = CPU_DEC_BC;

    run = runCPU(cpu, memory, 1);

    if (!testCPUState("DEC BC", testOutput, cpu, &expected) ||
        !testInt("DEC BC run result", testOutput, run, 2))
    {
        return 0;
    }

    cpu->b = 1; cpu->c = 0;
    expected.b = 0;
    expected.c = 0xFF;
    expected.pc = 2;
    
    run = runCPU(cpu, memory, 1);

    return testCPUState("DEC BC rollover", testOutput, cpu, &expected);
}


int testINC_C(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 1;
    expected.c = 1;

    memory->internalRam[0] = CPU_INC_C;

    run = runCPU(cpu, memory, 1);
    
    return 
        testCPUState("INC C", testOutput, cpu, &expected) &&
        testInt("INC C", testOutput, run, 1) &&
        1
    ;
}

int testDEC_C(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.f = 0x40;
    expected.pc = 1;
    expected.c = 0xFF;

    memory->internalRam[0] = CPU_DEC_C;

    run = runCPU(cpu, memory, 1);
    
    return 
        testCPUState("DEC C", testOutput, cpu, &expected) &
        testInt("DEC C", testOutput, run, 1) &&
        1
    ;
}


int testLD_C_d8(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 2;
    expected.c = 23;

    memory->internalRam[0] = CPU_LD_C_d8;
    memory->internalRam[1] = 23;

    run = runCPU(cpu, memory, 1);

    return 
        testCPUState("LD C d8", testOutput, cpu, &expected) &&
        testInt("LD C d8 run result", testOutput, run, 2) &&
        1
    ;
}

int testRRCA(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = 0xF0;
    cpu->a = 0x1;
    expected = *cpu;
    expected.pc = 1;
    expected.f = 0x10;
    expected.a = 0x80;

    memory->internalRam[0] = CPU_RRCA;
    memory->internalRam[1] = CPU_RRCA;

    run = runCPU(cpu, memory, 1);

    if (!testCPUState("RRCA", testOutput, cpu, &expected) ||
        !testInt("RRCA run result", testOutput, run, 1))
    {
        return 0;
    }
    
    cpu->a = 0x80;
    expected.pc = 2;
    expected.f = 0;
    expected.a = 0x40;
    run = runCPU(cpu, memory, 1);

    return 
        testCPUState("RRCA no carry", testOutput, cpu, &expected) &&
        testInt("RRCA run result", testOutput, run, 1) &&
        1
    ;
}

int run0x0Tests(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    return 
        testNOP(cpu, memory, testOutput) &&
        testLD_BC_d16(cpu, memory, testOutput) &&
        testLD_BC_A(cpu, memory, testOutput) &&
        testINC_BC(cpu, memory, testOutput) &&
        testINC_B(cpu, memory, testOutput) &&
        testDEC_B(cpu, memory, testOutput) &&
        testLD_B_d8(cpu, memory, testOutput) &&
        testRLCA(cpu, memory, testOutput) &&
        testLD_A16_SP(cpu, memory, testOutput) &&
        testADD_HL_BC(cpu, memory, testOutput) &&
        testLD_A_BC(cpu, memory, testOutput) &&
        testDEC_BC(cpu, memory, testOutput) &&
        testINC_C(cpu, memory, testOutput) &&
        testDEC_C(cpu, memory, testOutput) &&
        testLD_C_d8(cpu, memory, testOutput) &&
        testRRCA(cpu, memory, testOutput) &&
        1
    ;
}