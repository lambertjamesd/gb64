
#include "cpu_test.h"

int testJR_NZ(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 4;

    memory->internalRam[0] = CPU_JR_NZ;
    memory->internalRam[1] = 2;
    memory->internalRam[2] = CPU_NOP;
    memory->internalRam[3] = CPU_NOP;
    memory->internalRam[4] = CPU_JR_NZ;
    memory->internalRam[5] = (unsigned char)(-6);

    run = runCPUCPU(cpu, memory, 1);

    if (!testCPUState("JR NZ", testOutput, cpu, &expected) ||
        !testInt("JR NZ run result", testOutput, run, 3))
    {
        return 0;
    }
    
    expected.pc = 0;
    run = runCPUCPU(cpu, memory, 1);
    
    if (!testCPUState("JR NZ", testOutput, cpu, &expected) ||
        !testInt("JR NZ run result", testOutput, run, 3))
    {
        return 0;
    }

    cpu->f = GB_FLAGS_Z;
    expected.pc = 2;
    expected.f = GB_FLAGS_Z;
    run = runCPUCPU(cpu, memory, 1);

    return 
        testCPUState("JR NZ", testOutput, cpu, &expected) &&
        testInt("JR NZ run result", testOutput, run, 2) &&
        1
    ;
}

int testLD_HL_d16(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 3;
    expected.h = 13;
    expected.l = 17;

    memory->internalRam[0] = CPU_LD_HL_d16;
    memory->internalRam[1] = 17;
    memory->internalRam[2] = 13;

    run = runCPUCPU(cpu, memory, 1);

    return 
        testCPUState("LD HL d16", testOutput, cpu, &expected) &&
        testInt("LD HL d16 run result", testOutput, run, 3) &&
        1
    ;
}

int testLDI_HL_A(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);

    cpu->h = 0x81;
    cpu->l = 0x05;
    cpu->a = 63;

    expected = *cpu;
    expected.pc = 1;
    expected.l = 0x06;

    memory->internalRam[0] = CPU_LDI_HL_A;
    memory->internalRam[1] = CPU_LDI_HL_A;

    run = runCPUCPU(cpu, memory, 1);
    
    if (
        !testCPUState("LDI HL A", testOutput, cpu, &expected) ||
        !testInt("LDI HL A run result", testOutput, run, 2) ||
        !testInt("LDI HL A stored value", testOutput, memory->internalRam[0x105], 63))
    {
        return 0;
    }

    cpu->l = 0xFF;
    run = runCPUCPU(cpu, memory, 1);
    expected.pc = 2;
    expected.h = 0x82;
    expected.l = 0x00;

    return 
        testCPUState("LD HL A", testOutput, cpu, &expected) &&
        testInt("LD HL A run result", testOutput, run, 2) &&
        testInt("LD HL A stored value", testOutput, memory->internalRam[0x1FF], 63) &&
        1
    ;
}

int testINC_HL(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);

    cpu->h = 1;
    cpu->l = 3;

    expected = *cpu;
    expected.pc = 1;
    expected.h = 1;
    expected.l = 4;

    memory->internalRam[0] = CPU_INC_HL;
    memory->internalRam[1] = CPU_INC_HL;

    run = runCPUCPU(cpu, memory, 1);

    if (!testCPUState("INC HL", testOutput, cpu, &expected) ||
        !testInt("INC HL run result", testOutput, run, 2))
    {
        return 0;
    }

    cpu->h = 0; cpu->l = 0xFF;
    expected.h = 1;
    expected.l = 0;
    expected.pc = 2;
    
    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("INC HL rollover", testOutput, cpu, &expected);
}

int testINC_H(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 1;
    expected.h = 1;

    memory->internalRam[0] = CPU_INC_H;
    memory->internalRam[1] = CPU_INC_H;
    memory->internalRam[2] = CPU_INC_H;

    run = runCPUCPU(cpu, memory, 1);

    if (!testCPUState("INC H", testOutput, cpu, &expected) ||
        !testInt("INC H run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting zero flag
    cpu->h = 0xFF;
    cpu->f = 0x0;
    expected.f = 0xA0;
    expected.pc = 2;
    expected.h = 0;
    run = runCPUCPU(cpu, memory, 1);
    
    if (!testCPUState("INC H overflow", testOutput, cpu, &expected) ||
        !testInt("INC H run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting half flag
    cpu->h = 0x1F;
    cpu->f = 0;
    expected.f = 0x20;
    expected.pc = 3;
    expected.h = 0x20;
    run = runCPUCPU(cpu, memory, 1);
    
    return 
        testCPUState("INC H half carry", testOutput, cpu, &expected) &&
        testInt("INC H run result", testOutput, run, 1) &&
        1
    ;
}

int testDEC_H(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.f = 0x40;
    expected.pc = 1;
    expected.h = 0xFF;

    memory->internalRam[0] = CPU_DEC_H;
    memory->internalRam[1] = CPU_DEC_H;
    memory->internalRam[2] = CPU_DEC_H;

    run = runCPUCPU(cpu, memory, 1);

    if (!testCPUState("DEC H", testOutput, cpu, &expected) ||
        !testInt("DEC H run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting zero flag
    cpu->h = 0x1;
    cpu->f = 0;
    run = runCPUCPU(cpu, memory, 1);
    expected.f = 0xC0;
    expected.pc = 2;
    expected.h = 0;
    
    if (!testCPUState("DEC H zero", testOutput, cpu, &expected) ||
        !testInt("DEC H zero run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting half flag
    cpu->h = 0x10;
    cpu->f = 0;
    run = runCPUCPU(cpu, memory, 1);
    expected.f = 0x40;
    expected.pc = 3;
    expected.h = 0x0F;
    
    return 
        testCPUState("DEC H half carry", testOutput, cpu, &expected) &
        testInt("DEC H half carry run result", testOutput, run, 1) &&
        1
    ;
}

int testLD_H_d8(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 2;
    expected.h = 23;

    memory->internalRam[0] = CPU_LD_H_d8;
    memory->internalRam[1] = 23;

    run = runCPUCPU(cpu, memory, 1);

    return 
        testCPUState("LD H d8", testOutput, cpu, &expected) &&
        testInt("LD H d8 run result", testOutput, run, 2) &&
        1
    ;
}


int testDAA(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    return 1; // TODO
}

int testJR_Z(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = GB_FLAGS_Z;
    expected = *cpu;
    expected.pc = 4;

    memory->internalRam[0] = CPU_JR_Z;
    memory->internalRam[1] = 2;
    memory->internalRam[2] = CPU_NOP;
    memory->internalRam[3] = CPU_NOP;
    memory->internalRam[4] = CPU_JR_Z;
    memory->internalRam[5] = (unsigned char)(-6);

    run = runCPUCPU(cpu, memory, 1);

    if (!testCPUState("JR Z", testOutput, cpu, &expected) ||
        !testInt("JR Z run result", testOutput, run, 3))
    {
        return 0;
    }
    
    expected.pc = 0;
    run = runCPUCPU(cpu, memory, 1);
    
    if (!testCPUState("JR Z second", testOutput, cpu, &expected) ||
        !testInt("JR Z run result", testOutput, run, 3))
    {
        return 0;
    }

    cpu->f = 0;
    expected.pc = 2;
    expected.f = 0;
    run = runCPUCPU(cpu, memory, 1);

    return 
        testCPUState("JR Z third", testOutput, cpu, &expected) &&
        testInt("JR Z run result", testOutput, run, 2) &&
        1
    ;
}

int testADD_HL_HL(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->h = 0x02;
    cpu->l = 0x80;
    expected = *cpu;
    expected.pc = 1;
    expected.h = 0x05;
    expected.l = 0x00;
    expected.f = 0x0;

    memory->internalRam[0] = CPU_ADD_HL_HL;
    
    run = runCPUCPU(cpu, memory, 1);

    return 
        testCPUState("HL HL", testOutput, cpu, &expected) &&
        testInt("HL HL run result", testOutput, run, 2) &&
        1
    ;
}

int testLDI_A_HL(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->h = 0x21;
    cpu->l = 0xFF;
    expected = *cpu;
    expected.pc = 1;
    expected.a = 0xA3;
    expected.h = 0x22;
    expected.l = 0x00;

    memory->internalRam[0] = CPU_LDI_A_HL;
    memory->internalRam[0x1FF] = 0xA3;

    run = runCPUCPU(cpu, memory, 1);

    return 
        testCPUState("LDI A HL", testOutput, cpu, &expected) &&
        testInt("LDI A HL run result", testOutput, run, 2) &&
        1
    ;
}


int testDEC_HL(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);

    cpu->h = 1;
    cpu->l = 3;

    expected = *cpu;
    expected.pc = 1;
    expected.h = 1;
    expected.l = 2;

    memory->internalRam[0] = CPU_DEC_HL;
    memory->internalRam[1] = CPU_DEC_HL;

    run = runCPUCPU(cpu, memory, 1);

    if (!testCPUState("DEC HL", testOutput, cpu, &expected) ||
        !testInt("DEC HL run result", testOutput, run, 2))
    {
        return 0;
    }

    cpu->h = 1; cpu->l = 0;
    expected.h = 0;
    expected.l = 0xFF;
    expected.pc = 2;
    
    run = runCPUCPU(cpu, memory, 1);

    return testCPUState("DEC HL rollover", testOutput, cpu, &expected);
}


int testINC_L(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 1;
    expected.l = 1;

    memory->internalRam[0] = CPU_INC_L;

    run = runCPUCPU(cpu, memory, 1);
    
    return 
        testCPUState("INC L", testOutput, cpu, &expected) &&
        testInt("INC L", testOutput, run, 1) &&
        1
    ;
}

int testDEC_L(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.f = 0x40;
    expected.pc = 1;
    expected.l = 0xFF;

    memory->internalRam[0] = CPU_DEC_L;

    run = runCPUCPU(cpu, memory, 1);
    
    return 
        testCPUState("DEC E", testOutput, cpu, &expected) &
        testInt("DEC E", testOutput, run, 1) &&
        1
    ;
}


int testLD_L_d8(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 2;
    expected.l = 23;

    memory->internalRam[0] = CPU_LD_L_d8;
    memory->internalRam[1] = 23;

    run = runCPUCPU(cpu, memory, 1);

    return 
        testCPUState("LD E d8", testOutput, cpu, &expected) &&
        testInt("LD E d8 run result", testOutput, run, 2) &&
        1
    ;
}
int testCPL(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 1;
    cpu->a = 0x3E;
    expected.a = ~0x3E;
    expected.f = 0x60;

    memory->internalRam[0] = CPU_CPL;

    run = runCPUCPU(cpu, memory, 1);

    return 
        testCPUState("CPL d16", testOutput, cpu, &expected) &&
        testInt("CPL run result", testOutput, run, 1) &&
        1
    ;
}

int run0x2Tests(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    return 
        testJR_NZ(cpu, memory, testOutput) &&
        testLD_HL_d16(cpu, memory, testOutput) &&
        testLDI_HL_A(cpu, memory, testOutput) &&
        testINC_HL(cpu, memory, testOutput) &&
        testINC_H(cpu, memory, testOutput) &&
        testDEC_H(cpu, memory, testOutput) &&
        testLD_H_d8(cpu, memory, testOutput) &&
        testDAA(cpu, memory, testOutput) &&
        testJR_Z(cpu, memory, testOutput) &&
        testADD_HL_HL(cpu, memory, testOutput) &&
        testLDI_A_HL(cpu, memory, testOutput) &&
        testDEC_HL(cpu, memory, testOutput) &&
        testINC_L(cpu, memory, testOutput) &&
        testDEC_L(cpu, memory, testOutput) &&
        testLD_L_d8(cpu, memory, testOutput) &&
        testCPL(cpu, memory, testOutput) &&
        1
    ;
}