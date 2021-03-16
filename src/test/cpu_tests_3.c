
#include "cpu_test.h"

int testJR_NC(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 4;

    memory->internalRam[0] = CPU_JR_NC;
    memory->internalRam[1] = 2;
    memory->internalRam[2] = CPU_NOP;
    memory->internalRam[3] = CPU_NOP;
    memory->internalRam[4] = CPU_JR_NC;
    memory->internalRam[5] = (unsigned char)(-6);

    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("JR NC", testOutput, cpu, &expected) ||
        !testInt("JR NC run result", testOutput, run, 3))
    {
        return 0;
    }
    
    expected.pc = 0;
    run = runCPU(cpu, memory, 1, 0);
    
    if (!testCPUState("JR NC", testOutput, cpu, &expected) ||
        !testInt("JR NC run result", testOutput, run, 3))
    {
        return 0;
    }

    cpu->f = GB_FLAGS_C;
    expected.pc = 2;
    expected.f = GB_FLAGS_C;
    run = runCPU(cpu, memory, 1, 0);

    return 
        testCPUState("JR NC", testOutput, cpu, &expected) &&
        testInt("JR NC run result", testOutput, run, 2) &&
        1
    ;
}

int testLD_SP_d16(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 3;
    expected.sp = 0xD011;

    memory->internalRam[0] = CPU_LD_SP_d16;
    memory->internalRam[1] = 0x11;
    memory->internalRam[2] = 0xD0;

    run = runCPU(cpu, memory, 1, 0);

    return 
        testCPUState("LD SP d16", testOutput, cpu, &expected) &&
        testInt("LD SP d16 run result", testOutput, run, 3) &&
        1
    ;
}

int testLDD_HL_A(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);

    cpu->h = 0x81;
    cpu->l = 0x05;
    cpu->a = 63;

    expected = *cpu;
    expected.pc = 1;
    expected.l = 0x04;

    memory->internalRam[0] = CPU_LDD_HL_A;
    memory->internalRam[1] = CPU_LDD_HL_A;

    run = runCPU(cpu, memory, 1, 0);
    
    if (
        !testCPUState("LDD HL A", testOutput, cpu, &expected) ||
        !testInt("LDD HL A run result", testOutput, run, 2) ||
        !testInt("LDD HL A stored value", testOutput, memory->internalRam[0x105], 63))
    {
        return 0;
    }

    cpu->l = 0x00;
    run = runCPU(cpu, memory, 1, 0);
    expected.pc = 2;
    expected.h = 0x80;
    expected.l = 0xFF;

    return 
        testCPUState("LDD HL A", testOutput, cpu, &expected) &&
        testInt("LDD HL A run result", testOutput, run, 2) &&
        testInt("LDD HL A stored value", testOutput, memory->internalRam[0x100], 63) &&
        1
    ;
}

int testINC_SP(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);

    cpu->sp = 0x432;

    expected = *cpu;
    expected.pc = 1;
    expected.sp = 0x433;

    memory->internalRam[0] = CPU_INC_SP;
    memory->internalRam[1] = CPU_INC_SP;

    run = runCPU(cpu, memory, 1, 0);

    return testCPUState("INC SP", testOutput, cpu, &expected) &&
        testInt("INC SP run result", testOutput, run, 2);
}

int testINC_HL_ADDR(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->h = 0x81;
    cpu->l = 0x30;
    expected = *cpu;
    expected.pc = 1;

    memory->internalRam[0] = CPU_INC_HL_ADDR;
    memory->internalRam[0x130] = 0x31;

    run = runCPU(cpu, memory, 1, 0);
    
    return 
        testCPUState("INC (HL) half carry", testOutput, cpu, &expected) &&
        testInt("INC (HL) run result", testOutput, run, 3) &&
        testInt("INC (HL) memory", testOutput, memory->internalRam[0x130], 0x32) &&
        1
    ;
}

int testDEC_HL_ADDR(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->h = 0x81;
    cpu->l = 0x30;
    expected = *cpu;
    expected.pc = 1;
    expected.f = 0x40;

    memory->internalRam[0] = CPU_DEC_HL_ADDR;
    memory->internalRam[0x130] = 0x31;

    run = runCPU(cpu, memory, 1, 0);
    
    return 
        testCPUState("DEC (HL) half carry", testOutput, cpu, &expected) &&
        testInt("DEC (HL) run result", testOutput, run, 3) &&
        testInt("DEC (HL) memory", testOutput, memory->internalRam[0x130], 0x30) &&
        1
    ;
}

int testLD_HL_d8(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->h = 0x81;
    cpu->l = 0x24;
    expected = *cpu;
    expected.pc = 2;

    memory->internalRam[0] = CPU_LD_HL_d8;
    memory->internalRam[1] = 23;

    run = runCPU(cpu, memory, 1, 0);

    return 
        testCPUState("LD (HL) d8", testOutput, cpu, &expected) &&
        testInt("LD (HL) d8 run result", testOutput, run, 3) &&
        testInt("LD (HL) d8 memory", testOutput, memory->internalRam[0x124], 23) &&
        1
    ;
}


int testSCF(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = 0x80;
    expected = *cpu;
    expected.pc = 1;
    expected.f = 0x90;

    memory->internalRam[0] = CPU_SCF;

    run = runCPU(cpu, memory, 1, 0);

    return 
        testCPUState("SCF", testOutput, cpu, &expected) &&
        testInt("SCF run result", testOutput, run, 1) &&
        1
    ;
}

int testJR_C(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->f = GB_FLAGS_C;
    expected = *cpu;
    expected.pc = 4;

    memory->internalRam[0] = CPU_JR_C;
    memory->internalRam[1] = 2;
    memory->internalRam[2] = CPU_NOP;
    memory->internalRam[3] = CPU_NOP;
    memory->internalRam[4] = CPU_JR_C;
    memory->internalRam[5] = (unsigned char)(-6);

    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("JR C", testOutput, cpu, &expected) ||
        !testInt("JR C run result", testOutput, run, 3))
    {
        return 0;
    }
    
    expected.pc = 0;
    run = runCPU(cpu, memory, 1, 0);
    
    if (!testCPUState("JR C second", testOutput, cpu, &expected) ||
        !testInt("JR C run result", testOutput, run, 3))
    {
        return 0;
    }

    cpu->f = 0;
    expected.pc = 2;
    expected.f = 0;
    run = runCPU(cpu, memory, 1, 0);

    return 
        testCPUState("JR C third", testOutput, cpu, &expected) &&
        testInt("JR C run result", testOutput, run, 2) &&
        1
    ;
}

int testADD_HL_SP(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->h = 0x02;
    cpu->l = 0x80;
    cpu->sp = 0x8081;
    expected = *cpu;
    expected.pc = 1;
    expected.h = 0x83;
    expected.l = 0x01;
    expected.f = 0x0;

    memory->internalRam[0] = CPU_ADD_HL_SP;

    run = runCPU(cpu, memory, 1, 0);

    return 
        testCPUState("HL SP half", testOutput, cpu, &expected) &&
        testInt("HL SP run result", testOutput, run, 2) &&
        1
    ;
}

int testLDD_A_HL(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->h = 0x21;
    cpu->l = 0x00;
    expected = *cpu;
    expected.pc = 1;
    expected.a = 0xA3;
    expected.h = 0x20;
    expected.l = 0xFF;

    memory->internalRam[0] = CPU_LDD_A_HL;
    memory->internalRam[0x100] = 0xA3;

    run = runCPU(cpu, memory, 1, 0);

    return 
        testCPUState("LDD A HL", testOutput, cpu, &expected) &&
        testInt("LDD A HL run result", testOutput, run, 2) &&
        1
    ;
}


int testDEC_SP(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);

    cpu->sp = 0x1234;

    expected = *cpu;
    expected.pc = 1;
    expected.sp = 0x1233;

    memory->internalRam[0] = CPU_DEC_SP;

    run = runCPU(cpu, memory, 1, 0);

    return testCPUState("DEC SP", testOutput, cpu, &expected) &&
        testInt("DEC SP run result", testOutput, run, 2);
}


int testINC_A(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 1;
    expected.a = 1;

    memory->internalRam[0] = CPU_INC_A;

    run = runCPU(cpu, memory, 1, 0);
    
    return 
        testCPUState("INC A", testOutput, cpu, &expected) &&
        testInt("INC A", testOutput, run, 1) &&
        1
    ;
}

int testDEC_A(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.f = 0x60;
    expected.pc = 1;
    expected.a = 0xFF;

    memory->internalRam[0] = CPU_DEC_A;

    run = runCPU(cpu, memory, 1, 0);
    
    return 
        testCPUState("DEC A", testOutput, cpu, &expected) &
        testInt("DEC A", testOutput, run, 1) &&
        1
    ;
}


int testLD_A_d8(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 2;
    expected.a = 23;

    memory->internalRam[0] = CPU_LD_A_d8;
    memory->internalRam[1] = 23;

    run = runCPU(cpu, memory, 1, 0);

    return 
        testCPUState("LD A d8", testOutput, cpu, &expected) &&
        testInt("LD A d8 run result", testOutput, run, 2) &&
        1
    ;
}

int testCCF(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 1;
    cpu->f = 0x80;
    expected.f = 0x90;

    memory->internalRam[0] = CPU_CCF;

    run = runCPU(cpu, memory, 1, 0);

    return 
        testCPUState("CCF d16", testOutput, cpu, &expected) &&
        testInt("CCF run result", testOutput, run, 1) &&
        1
    ;
}

int run0x3Tests(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    return 
        testJR_NC(cpu, memory, testOutput) &&
        testLD_SP_d16(cpu, memory, testOutput) &&
        testLDD_HL_A(cpu, memory, testOutput) &&
        testINC_SP(cpu, memory, testOutput) &&
        testINC_HL_ADDR(cpu, memory, testOutput) &&
        testDEC_HL_ADDR(cpu, memory, testOutput) &&
        testLD_HL_d8(cpu, memory, testOutput) &&
        testSCF(cpu, memory, testOutput) &&
        testJR_C(cpu, memory, testOutput) &&
        testADD_HL_SP(cpu, memory, testOutput) &&
        testLDD_A_HL(cpu, memory, testOutput) &&
        testDEC_SP(cpu, memory, testOutput) &&
        testINC_A(cpu, memory, testOutput) &&
        testDEC_A(cpu, memory, testOutput) &&
        testLD_A_d8(cpu, memory, testOutput) &&
        testCCF(cpu, memory, testOutput) &&
        1
    ;
}