
#ifndef _DEBUGGER_H
#define _DEBUGGER_H

#include "memory_map.h"
#include "cpu.h"
#include "menu.h"
#include "../controller.h"
#include "bool.h"

enum DebuggerMenuIndices {
    DebuggerMenuIndicesInstructions,
    DebuggerMenuIndicesMemoryAddress,
    DebuggerMenuIndicesMemoryValues,
    DebuggerMenuIndicesCPUState,
    DebuggerMenuIndicesEditValue,
    DebuggerMenuIndicesCount,
};


#define DEBUGGER_FONT_W     8

#define CPU_STATE_X         8
#define CPU_STATE_Y         10

#define MEMORY_BLOCK_ROWS   12
#define MEMORY_BLOCK_COLS   4

#define MEMORY_GRID_X   8
#define MEMORY_GRID_Y   80
#define DEBUG_MENU_ROW_HEIGHT 10

#define MEMORY_VALUES_X         48
#define MEMORY_VALUES_SPACING   24

struct EditValueMenuState {
    struct MenuItem* returnTo;
    u16 x;
    u16 y;
    u16 nibbleWidth; // number of nibbles
    u16 currentNibble;
    u16 currentValue;
    u16 confirmEdit; // 1 if edit is accepted
};

struct DebuggerMenuState {
    int cursorX;
    int cursorY;
    bool isDebugging;
};

struct MemoryAddressMenuItem {
    u16 addressStart;
    struct DebuggerMenuState* menuState;
    struct MenuItem* editMenuItem;
};

struct MemoryValueMenuItem {
    u8 values[MEMORY_BLOCK_ROWS * MEMORY_BLOCK_COLS];
    struct DebuggerMenuState* menuState;
    struct Memory* memory;
    struct MemoryAddressMenuItem* addressGUI;
    u16 addressStart;
    struct MenuItem* editMenuItem;
};

#define MI_INSTRUCTIONS_Y           10
#define MI_INSTRUCTIONS_X           152
#define MI_INSTRUCTIONS_NAME_X      192
#define MI_INSTRUCTIONS_LINE_COUNT  20
#define MI_INSTRUCTION_Y_OFFSET     8
#define MI_INSTRUCTION_ADDR_COL     3
#define MI_INSTRUCTION_NAME_COL     4

struct InstructionLine {
    u16 address;
    u8 instruction[4];
    struct Breakpoint* breakpoint;
};

struct InstructionsMenuItem {
    u16 startAddress;
    struct InstructionLine instructions[MI_INSTRUCTIONS_LINE_COUNT];
    struct DebuggerMenuState* menuState;
    struct CPUState* cpu;
    struct Memory* memory;
    struct MenuItem* editMenuItem;
};

struct DebuggerMenu {
    struct MenuState menu;
    struct MenuItem menuItems[DebuggerMenuIndicesCount];
    struct DebuggerMenuState state;
    struct MemoryAddressMenuItem memoryAddresses;
    struct MemoryValueMenuItem memoryValues;
    struct InstructionsMenuItem instructionMenuItem;
    struct EditValueMenuState editMenu;
};

extern struct DebuggerMenu gDebugMenu;

void writeMemoryDirect(struct Memory* memory, u16 address, u8 value);
u8 readMemoryDirect(struct Memory* memory, u16 address);

struct Breakpoint* addBreakpoint(struct Memory* memory, u16 address, enum BreakpointType type);
void removeBreakpoint(struct Memory* memory, u16 address);
void reapplyBreakpoint(struct Breakpoint* breakpoint);

u8 useDebugger(struct CPUState* cpu, struct Memory* memory);

void initDebugMenu(struct DebuggerMenu* menu, struct CPUState* cpu, struct Memory* memory);

#endif