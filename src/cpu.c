#include "cpu.h"

void initializeCPU(struct CPUState* state)
{
    state->a = 0; state->f = 0; state->b = 0; state->c = 0;
    state->d = 0; state->e = 0; state->h = 0; state->l = 0;
    state->sp = 0; state->pc = 0;
    state->stopReason = STOP_REASON_NONE;
    state->interrupts = 0;
    state->nextInterrupt = 0;
    state->runUntilNextFrame = 0;
    state->cyclesRun = 0;
    state->nextTimerTrigger = ~0;
    state->nextScreenTrigger = ~0;
    state->nextInterruptTrigger = ~0;
    state->nextStoppingPoint = CPU_STOPPING_POINT_COUNT * sizeof(struct CPUStoppingPoint);
}