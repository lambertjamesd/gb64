
#ifndef _CPU_H
#define _CPU_H

#include "memory_map.h"

enum STOP_REASON {
    STOP_REASON_NONE,
    STOP_REASON_STOP,
    STOP_REASON_HALT,
    STOP_REASON_INTERRUPT_RET,
    STOP_REASON_ERROR,
};

enum GB_FLAGS {
    GB_FLAGS_Z = 0x80,
    GB_FLAGS_N = 0x40,
    GB_FLAGS_H = 0x20,
    GB_FLAGS_C = 0x10,
};

enum GB_INTERRUPTS {
    GB_INTERRUPTS_V_BLANK   = 0x01,
    GB_INTERRUPTS_LCDC      = 0x02,
    GB_INTERRUPTS_TIMER     = 0x04,
    GB_INTERRUPTS_SERIAL    = 0x08,
    GB_INTERRUPTS_INPUT     = 0x10,
    GB_INTERRUPTS_ENABLED   = 0x80,
};

enum CPUStoppingPointType {
    CPUStoppingPointTypeNone,
    CPUStoppingPointTypeScreenMode0,
    CPUStoppingPointTypeScreenMode1,
    CPUStoppingPointTypeScreenMode2,
    CPUStoppingPointTypeScreenMode3,
    CPUStoppingPointTypeTimerReset,
    CPUStoppingPointTypeInterrupt,
    CPUStoppingPointTypeExit,
    CPUStoppingPointTypeDMA,
    CPUStoppingPointTypeDebugger,
};

struct CPUStoppingPoint {
    unsigned long cycleTime:24;
    unsigned long stoppingPointType:8;
};

#define CPU_STOPPING_POINT_AS_LONG(value) *((unsigned long*)&value)

#define CPU_STOPPING_POINT_COUNT    0x10
#define MAX_CYCLE_TIME  0x00800000

struct CPUState {
    unsigned char a;
    unsigned char f;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    unsigned char e;
    unsigned char h;
    unsigned char l;
    unsigned short sp;
    unsigned short pc;
    unsigned char stopReason;
    unsigned char interrupts;
    unsigned char gbc;
    unsigned char runUntilNextFrame;
    unsigned long cyclesRun;
    unsigned long nextTimerTrigger;
    unsigned long nextScreenTrigger;
    unsigned long nextInterruptTrigger;
    // cpu timer uneffected by speed switching
    unsigned long unscaledCyclesRun; 
    unsigned long nextStoppingPoint;
    struct CPUStoppingPoint stoppingPoints[CPU_STOPPING_POINT_COUNT];
};

#define RUN_CPU_FLAGS_RENDER        0x1

extern int runCPU(struct CPUState* state, struct Memory* memory, int cyclesToRun, int flags);
extern void initializeCPU(struct CPUState* state);
extern void addStoppingPoint(struct CPUState* state, struct CPUStoppingPoint stoppingPoint);

// We don't want CYCLES_RUN
// to overflow while emulating the CPU
// since CPU_STATE_NEXT_TIMER should always
// be >= CYCLES_RUN
// to prevent this, when exiting the
// emulation we check if CYCLES_RUN is
// above 0x80000000 if it is we 
// decrement both CYCLES_RUN and
// by that much CPU_STATE_NEXT_TIMER
extern void adjustCPUTimer(struct CPUState* state);

#endif