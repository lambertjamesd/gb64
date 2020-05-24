#include "z80_test.h"

int runTimerInterruptTest(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    initializeZ80(z80);
    z80->a = 0;
    z80->interrupts = GB_INTERRUPTS_ENABLED;
    z80->sp = 0xFFFE;

    WRITE_REGISTER_DIRECT(memory, REG_INT_ENABLED, GB_INTERRUPTS_TIMER);
    WRITE_REGISTER_DIRECT(memory, REG_TIMA, 0xFF);
    WRITE_REGISTER_DIRECT(memory, REG_TAC, 0x5);

    memory->internalRam[0] = Z80_NOP;
    memory->internalRam[1] = Z80_JR;
    memory->internalRam[2] = -3;

    memory->internalRam[0x50] = Z80_INC_A;
    memory->internalRam[0x51] = Z80_RETI;

    runZ80CPU(z80, memory, 5);

    if (!testInt("INT TIMER", testOutput, 
        z80->pc, 
        0x51) ||
        !testInt("INT TIMER interrupts", testOutput, 
        z80->interrupts, 
        0)
    )
    {
        return 0;
    }
    
    runZ80CPU(z80, memory, 4);

    return testInt("INT TIMER CALLED", testOutput, 
        z80->a, 
        1) &&
        testInt("INT TIMER CALLED interrupts", testOutput, 
        z80->interrupts, 
        GB_INTERRUPTS_ENABLED)
    ;
}

int runInterruptTests(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    return
        runTimerInterruptTest(z80, memory, testOutput) &&
    1;
}