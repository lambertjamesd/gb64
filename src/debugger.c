#include "debugger.h"


u8* getMemoryAddress(struct Memory* memory, u16 address)
{
    if (address < MISC_START)
    {
        return (u8*)memory->memoryMap[address >> 12] + (address & 0xFFF);
    }
    else
    {
        return memory->miscBytes + (address - MISC_START);
    }
}

void removeBreakpointDirect(struct Breakpoint* breakpoint)
{
    if (breakpoint->breakpointType != BreakpointTypeNone)
    {
        *breakpoint->memoryAddress = breakpoint->existingInstruction;
        breakpoint->memoryAddress = 0;
        breakpoint->address = 0;
        breakpoint->existingInstruction = 0;
        breakpoint->breakpointType = BreakpointTypeNone;
    }
}

void writeMemoryDirect(struct Memory* memory, u16 address, u8 value)
{
    *getMemoryAddress(memory, address) = value;
}

u8 readMemoryDirect(struct Memory* memory, u16 address)
{
    return *getMemoryAddress(memory, address);
}

void insertBreakpoint(struct Memory* memory, u16 address, enum BreakpointType type, int breakpointIndex)
{
    if (memory->breakpoints[breakpointIndex].breakpointType != BreakpointTypeNone)
    {
        removeBreakpointDirect(&memory->breakpoints[breakpointIndex]);
    }

    memory->breakpoints[breakpointIndex].memoryAddress = getMemoryAddress(memory, address);
    memory->breakpoints[breakpointIndex].address = address;
    memory->breakpoints[breakpointIndex].existingInstruction = *memory->breakpoints[breakpointIndex].memoryAddress;
    memory->breakpoints[breakpointIndex].existingInstruction = type;

    *memory->breakpoints[breakpointIndex].memoryAddress = DEBUG_INSTRUCTION;
}

void addBreakpoint(struct Memory* memory, u16 address, enum BreakpointType type)
{
    int index;
    for (index = 0; index < USER_BREAK_POINTS; ++index)
    {
        if (memory->breakpoints[index].breakpointType == BreakpointTypeNone)
        {
            insertBreakpoint(memory, address, BreakpointTypeUser, index);
            break;
        }
    }
}

void removeBreakpoint(struct Memory* memory, u16 address)
{
    int index;

    for (index = 0; index < BREAK_POINT_COUNT; ++index)
    {
        if (memory->breakpoints[index].address == address)
        {
            removeBreakpointDirect(&memory->breakpoints[index]);
        }
    }
}

void useDebugger(struct CPUState* cpu, struct Memory* memory)
{
    int paused = 1;

    while (paused)
    {

    }
}