
#include "cpu_test.h"

int testSTOP(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 2;
    expected.stopReason = STOP_REASON_STOP;

    memory->internalRam[0] = CPU_STOP;

    run = runCPU(cpu, memory, 10, 0); // try to run many instructions

    return 
        testCPUState("STOP", testOutput, cpu, &expected) &&
        testInt("STOP run result", testOutput, run, 1) &&
        1
    ;
}

int testLD_DE_d16(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 3;
    expected.d = 13;
    expected.e = 17;

    memory->internalRam[0] = CPU_LD_DE_d16;
    memory->internalRam[1] = 17;
    memory->internalRam[2] = 13;

    run = runCPU(cpu, memory, 1, 0);

    return 
        testCPUState("LD DE d16", testOutput, cpu, &expected) &&
        testInt("LD DE d16 run result", testOutput, run, 3) &&
        1
    ;
}

int testLD_DE_A(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);

    cpu->d = 0x81;
    cpu->e = 0x05;
    cpu->a = 63;

    expected = *cpu;
    expected.pc = 1;

    memory->internalRam[0] = CPU_LD_DE_A;

    run = runCPU(cpu, memory, 1, 0);

    return 
        testCPUState("LD DE A", testOutput, cpu, &expected) &&
        testInt("LD DE A run result", testOutput, run, 2) &&
        testInt("LD DE A stored value", testOutput, memory->internalRam[0x105], 63) &&
        1
    ;
}

int testINC_DE(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);

    cpu->d = 1;
    cpu->e = 3;

    expected = *cpu;
    expected.pc = 1;
    expected.d = 1;
    expected.e = 4;

    memory->internalRam[0] = CPU_INC_DE;
    memory->internalRam[1] = CPU_INC_DE;

    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("INC DE", testOutput, cpu, &expected) ||
        !testInt("INC DE run result", testOutput, run, 2))
    {
        return 0;
    }

    cpu->d = 0; cpu->e = 0xFF;
    expected.d = 1;
    expected.e = 0;
    expected.pc = 2;
    
    run = runCPU(cpu, memory, 1, 0);

    return testCPUState("INC DE rollover", testOutput, cpu, &expected);
}

int testINC_D(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 1;
    expected.d = 1;

    memory->internalRam[0] = CPU_INC_D;
    memory->internalRam[1] = CPU_INC_D;
    memory->internalRam[2] = CPU_INC_D;

    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("INC D", testOutput, cpu, &expected) ||
        !testInt("INC D run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting zero flag
    cpu->d = 0xFF;
    cpu->f = 0x0;
    expected.f = 0xA0;
    expected.pc = 2;
    expected.d = 0;
    run = runCPU(cpu, memory, 1, 0);
    
    if (!testCPUState("INC D overflow", testOutput, cpu, &expected) ||
        !testInt("INC D run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting half flag
    cpu->d = 0x1F;
    cpu->f = 0;
    expected.f = 0x20;
    expected.pc = 3;
    expected.d = 0x20;
    run = runCPU(cpu, memory, 1, 0);
    
    return 
        testCPUState("INC D half carry", testOutput, cpu, &expected) &&
        testInt("INC D run result", testOutput, run, 1) &&
        1
    ;
}

int testDEC_D(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.f = 0x60;
    expected.pc = 1;
    expected.d = 0xFF;

    memory->internalRam[0] = CPU_DEC_D;
    memory->internalRam[1] = CPU_DEC_D;
    memory->internalRam[2] = CPU_DEC_D;

    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("DEC D", testOutput, cpu, &expected) ||
        !testInt("DEC D run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting zero flag
    cpu->d = 0x1;
    cpu->f = 0;
    run = runCPU(cpu, memory, 1, 0);
    expected.f = 0xC0;
    expected.pc = 2;
    expected.d = 0;
    
    if (!testCPUState("DEC D zero", testOutput, cpu, &expected) ||
        !testInt("DEC D zero run result", testOutput, run, 1))
    {
        return 0;
    }

    // test setting half flag
    cpu->d = 0x10;
    cpu->f = 0;
    run = runCPU(cpu, memory, 1, 0);
    expected.f = 0x60;
    expected.pc = 3;
    expected.d = 0x0F;
    
    return 
        testCPUState("DEC D half carry", testOutput, cpu, &expected) &
        testInt("DEC D half carry run result", testOutput, run, 1) &&
        1
    ;
}

int testLD_D_d8(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 2;
    expected.d = 23;

    memory->internalRam[0] = CPU_LD_D_d8;
    memory->internalRam[1] = 23;

    run = runCPU(cpu, memory, 1, 0);

    return 
        testCPUState("LD D d8", testOutput, cpu, &expected) &&
        testInt("LD D d8 run result", testOutput, run, 2) &&
        1
    ;
}

int testRLA(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 1;
    expected.a = 0x3;
    cpu->a = 0x1;
    cpu->f = 0xF0;

    memory->internalRam[0] = CPU_RLA;
    memory->internalRam[1] = CPU_RLA;

    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("RLA", testOutput, cpu, &expected) ||
        !testInt("RLA run result", testOutput, run, 1))
    {
        return 0;
    }

    cpu->a = 0x80;
    expected.pc = 2;
    expected.a = 0x0;
    expected.f = 0x10;
    run = runCPU(cpu, memory, 1, 0);
    
    return 
        testCPUState("RLA set carry", testOutput, cpu, &expected) &&
        testInt("RLA run result", testOutput, run, 1) &&
        1
    ;
}

int testJR(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 4;

    memory->internalRam[0] = CPU_JR;
    memory->internalRam[1] = 2;
    memory->internalRam[2] = CPU_NOP;
    memory->internalRam[3] = CPU_NOP;
    memory->internalRam[4] = CPU_JR;
    memory->internalRam[5] = (unsigned char)(-6);

    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("JR", testOutput, cpu, &expected) ||
        !testInt("JR run result", testOutput, run, 3))
    {
        return 0;
    }
    
    expected.pc = 0;
    run = runCPU(cpu, memory, 1, 0);

    return 
        testCPUState("JR", testOutput, cpu, &expected) &&
        testInt("JR run result", testOutput, run, 3) &&
        1
    ;
}

int testADD_HL_DE(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->h = 0x02;
    cpu->l = 0x80;
    cpu->d = 0x80;
    cpu->e = 0x81;
    expected = *cpu;
    expected.pc = 1;
    expected.h = 0x83;
    expected.l = 0x01;
    expected.f = 0x0;

    memory->internalRam[0] = CPU_ADD_HL_DE;
    memory->internalRam[1] = CPU_ADD_HL_DE;
    memory->internalRam[2] = CPU_ADD_HL_DE;

    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("HL DE", testOutput, cpu, &expected) ||
        !testInt("HL DE run result", testOutput, run, 2))
    {
        return 0;
    }
    
    cpu->d = 0xD;
    cpu->e = 0;
    expected.pc = 2;
    expected.h = 0x90;
    expected.l = 0x01;
    expected.d = 0xD;
    expected.e = 0;
    expected.f = 0x20;
    
    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("HL DE", testOutput, cpu, &expected) ||
        !testInt("HL DE run result", testOutput, run, 2))
    {
        return 0;
    }

    cpu->d = 0x70;
    run = runCPU(cpu, memory, 1, 0);
    expected.pc = 3;
    expected.h = 0x00;
    expected.l = 0x01;
    expected.d = 0x70;
    expected.f = 0x10;

    return 
        testCPUState("HL DE half", testOutput, cpu, &expected) &&
        testInt("HL DE run result", testOutput, run, 2) &&
        1
    ;
}

int testLD_A_DE(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    cpu->d = 0x21;
    cpu->e = 0x30;
    expected = *cpu;
    expected.pc = 1;
    expected.a = 0xA3;

    memory->internalRam[0] = CPU_LD_A_DE;
    memory->internalRam[1] = 23;
    memory->internalRam[0x130] = 0xA3;

    run = runCPU(cpu, memory, 1, 0);

    return 
        testCPUState("LD A DE", testOutput, cpu, &expected) &&
        testInt("LD A DE run result", testOutput, run, 2) &&
        1
    ;
}


int testDEC_DE(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);

    cpu->d = 1;
    cpu->e = 3;

    expected = *cpu;
    expected.pc = 1;
    expected.d = 1;
    expected.e = 2;

    memory->internalRam[0] = CPU_DEC_DE;
    memory->internalRam[1] = CPU_DEC_DE;

    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("DEC DE", testOutput, cpu, &expected) ||
        !testInt("DEC DE run result", testOutput, run, 2))
    {
        return 0;
    }

    cpu->d = 1; cpu->e = 0;
    expected.d = 0;
    expected.e = 0xFF;
    expected.pc = 2;
    
    run = runCPU(cpu, memory, 1, 0);

    return testCPUState("DEC DE rollover", testOutput, cpu, &expected);
}


int testINC_E(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 1;
    expected.e = 1;

    memory->internalRam[0] = CPU_INC_E;

    run = runCPU(cpu, memory, 1, 0);
    
    return 
        testCPUState("INC E", testOutput, cpu, &expected) &&
        testInt("INC E", testOutput, run, 1) &&
        1
    ;
}

int testDEC_E(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.f = 0x60;
    expected.pc = 1;
    expected.e = 0xFF;

    memory->internalRam[0] = CPU_DEC_E;

    run = runCPU(cpu, memory, 1, 0);
    
    return 
        testCPUState("DEC E", testOutput, cpu, &expected) &
        testInt("DEC E", testOutput, run, 1) &&
        1
    ;
}


int testLD_E_d8(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 2;
    expected.e = 23;

    memory->internalRam[0] = CPU_LD_E_d8;
    memory->internalRam[1] = 23;

    run = runCPU(cpu, memory, 1, 0);

    return 
        testCPUState("LD E d8", testOutput, cpu, &expected) &&
        testInt("LD E d8 run result", testOutput, run, 2) &&
        1
    ;
}

int testRRA(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 1;
    expected.a = 0xC0;
    cpu->a = 0x80;
    cpu->f = 0xF0;

    memory->internalRam[0] = CPU_RRA;
    memory->internalRam[1] = CPU_RRA;

    run = runCPU(cpu, memory, 1, 0);

    if (!testCPUState("RRA", testOutput, cpu, &expected) ||
        !testInt("RRA run result", testOutput, run, 1))
    {
        return 0;
    }

    cpu->a = 0x01;
    expected.pc = 2;
    expected.a = 0x0;
    expected.f = 0x10;
    run = runCPU(cpu, memory, 1, 0);
    
    return 
        testCPUState("RRA set carry", testOutput, cpu, &expected) &&
        testInt("RRA run result", testOutput, run, 1) &&
        1
    ;
}

int run0x1Tests(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    return 
        // testSTOP(cpu, memory, testOutput) &&
        testLD_DE_d16(cpu, memory, testOutput) &&
        testLD_DE_A(cpu, memory, testOutput) &&
        testINC_DE(cpu, memory, testOutput) &&
        testINC_D(cpu, memory, testOutput) &&
        testDEC_D(cpu, memory, testOutput) &&
        testLD_D_d8(cpu, memory, testOutput) &&
        testRLA(cpu, memory, testOutput) &&
        testJR(cpu, memory, testOutput) &&
        testADD_HL_DE(cpu, memory, testOutput) &&
        testLD_A_DE(cpu, memory, testOutput) &&
        testDEC_DE(cpu, memory, testOutput) &&
        testINC_E(cpu, memory, testOutput) &&
        testDEC_E(cpu, memory, testOutput) &&
        testLD_E_d8(cpu, memory, testOutput) &&
        testRRA(cpu, memory, testOutput) &&
        1
    ;
}