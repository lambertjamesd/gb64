#include "cpu_test.h"

unsigned char timerInterruptProgram[] = {
    // enable timer interrupt
    CPU_LD_A_d8,
    GB_INTERRUPTS_TIMER,
    CPU_LDH_a8_A,
    REG_INT_ENABLED & 0xFF,

    // set TIMA to 0xFF
    CPU_LD_A_d8,
    0xFF,
    CPU_LDH_a8_A,
    REG_TIMA & 0xFF,

    // set TAC to 0x5
    CPU_LD_A_d8,
    0x5,
    CPU_LDH_a8_A,
    REG_TAC & 0xFF,

    CPU_LD_A_d8,
    0,
    
    // infinate loop
    CPU_NOP,
    CPU_JR,
    -3
};

int runTimerInterruptTest(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    initializeCPU(cpu);
    cpu->a = 0;
    cpu->interrupts = GB_INTERRUPTS_ENABLED;
    cpu->sp = 0xFFFE;

    LOAD_PROGRAM(memory->internalRam, timerInterruptProgram);

    memory->internalRam[0x50] = CPU_INC_A;
    memory->internalRam[0x51] = CPU_RETI;

    runCPUCPU(cpu, memory, 19);

    if (!testInt("INT TIMER", testOutput, 
        cpu->pc, 
        0x50) ||
        !testInt("INT TIMER interrupts", testOutput, 
        cpu->interrupts, 
        0)
    )
    {
        return 0;
    }
    
    runCPUCPU(cpu, memory, 4);

    if (!testInt("INT TIMER CALLED", testOutput, 
        cpu->a, 
        1) ||
        !testInt("INT TIMER CALLED interrupts", testOutput, 
        cpu->interrupts, 
        GB_INTERRUPTS_ENABLED)
    )
    {
        return 0;
    }

    runCPUCPU(cpu, memory, 0x400);

    return testInt("INT TIMER TWICE", testOutput, 
        cpu->a, 
        2);
}

int runInterruptTests(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    return
        runTimerInterruptTest(cpu, memory, testOutput) &&
    1;
}