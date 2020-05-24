#include "z80_test.h"

unsigned char timerInterruptProgram[] = {
    // enable timer interrupt
    Z80_LD_A_d8,
    GB_INTERRUPTS_TIMER,
    Z80_LDH_a8_A,
    REG_INT_ENABLED & 0xFF,

    // set TIMA to 0xFF
    Z80_LD_A_d8,
    0xFF,
    Z80_LDH_a8_A,
    REG_TIMA & 0xFF,

    // set TAC to 0x5
    Z80_LD_A_d8,
    0x5,
    Z80_LDH_a8_A,
    REG_TAC & 0xFF,

    Z80_LD_A_d8,
    0,
    
    // infinate loop
    Z80_NOP,
    Z80_JR,
    -3
};

int runTimerInterruptTest(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    initializeZ80(z80);
    z80->a = 0;
    z80->interrupts = GB_INTERRUPTS_ENABLED;
    z80->sp = 0xFFFE;

    LOAD_PROGRAM(memory->internalRam, timerInterruptProgram);

    memory->internalRam[0x50] = Z80_INC_A;
    memory->internalRam[0x51] = Z80_RETI;

    runZ80CPU(z80, memory, 19);

    if (!testInt("INT TIMER", testOutput, 
        z80->pc, 
        0x50) ||
        !testInt("INT TIMER interrupts", testOutput, 
        z80->interrupts, 
        0)
    )
    {
        return 0;
    }
    
    runZ80CPU(z80, memory, 4);

    if (!testInt("INT TIMER CALLED", testOutput, 
        z80->a, 
        1) ||
        !testInt("INT TIMER CALLED interrupts", testOutput, 
        z80->interrupts, 
        GB_INTERRUPTS_ENABLED)
    )
    {
        return 0;
    }

    runZ80CPU(z80, memory, 0x400);

    return testInt("INT TIMER TWICE", testOutput, 
        z80->a, 
        2);
}

int runInterruptTests(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    return
        runTimerInterruptTest(z80, memory, testOutput) &&
    1;
}