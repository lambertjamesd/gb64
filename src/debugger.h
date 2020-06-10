
#ifndef _DEBUGGER_H
#define _DEBUGGER_H

#include "memory_map.h"
#include "cpu.h"
#include "menu.h"
#include "../controller.h"

enum DebuggerMenuIndices {
    DebuggerMenuIndicesMemoryAddress,
    DebuggerMenuIndicesMemoryValues,
    DebuggerMenuIndicesCPUState,
    DebuggerMenuIndicesInstructions,
    DebuggerMenuIndicesPC,
    DebuggerMenuIndicesCount,
};

#define CPU_STATE_X         8
#define CPU_STATE_Y         8

#define MEMORY_BLOCK_ROWS   12
#define MEMORY_BLOCK_COLS   4

#define MEMROY_GRID_X   8
#define MEMORY_GRID_Y   80
#define DEBUG_MENU_ROW_HEIGHT 10

#define MEMORY_VALUES_X         48
#define MEMORY_VALUES_SPACING   24

struct DebuggerMenuState {
    int cursorX;
    int cursorY;
};

struct MemoryValueMenuItem {
    u8 values[MEMORY_BLOCK_ROWS * MEMORY_BLOCK_COLS];
    struct DebuggerMenuState* menuState;
    struct Memory* memory;
};

struct MemoryAddressMenuItem {
    u16 address[MEMORY_BLOCK_ROWS];
    struct MemoryValueMenuItem* values;
    struct DebuggerMenuState* menuState;
    struct Memory* memory;
};

struct DebuggerMenu {
    struct MenuState menu;
    struct MenuItem menuItems[DebuggerMenuIndicesCount];
    struct DebuggerMenuState state;
    struct MemoryAddressMenuItem memoryAddresses;
    struct MemoryValueMenuItem memoryValues;
};

void writeMemoryDirect(struct Memory* memory, u16 address, u8 value);
u8 readMemoryDirect(struct Memory* memory, u16 address);

void addBreakpoint(struct Memory* memory, u16 address, enum BreakpointType type);
void removeBreakpoint(struct Memory* memory, u16 address);

void useDebugger(struct CPUState* cpu, struct Memory* memory);

void initDebugMenu(struct DebuggerMenu* menu, struct CPUState* cpu, struct Memory* memory);

#endif