
#ifndef _DEBUGGER_H
#define _DEBUGGER_H

#include "memory_map.h"
#include "cpu.h"

void writeMemoryDirect(struct Memory* memory, u16 address, u8 value);
u8 readMemoryDirect(struct Memory* memory, u16 address);

void addBreakpoint(struct Memory* memory, u16 address, enum BreakpointType type);
void removeBreakpoint(struct Memory* memory, u16 address);

void useDebugger(struct CPUState* cpu, struct Memory* memory);

#endif