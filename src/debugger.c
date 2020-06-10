#include <ultra64.h>
#include "debugger.h"
#include "debug_out.h"
#include "../memory.h"
#include "../render.h"

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
    OSContPad	**pad;
    struct DebuggerMenu menu;
    int paused = 1;
    int lastButton = U_CBUTTONS;

    initDebugMenu(&menu, cpu, memory);

    while (paused)
    {
		pad = ReadController(0);

        menuStateHandleInput(&menu.menu, pad[0]);
    
		preRenderFrame(1);
		renderDebugLog();

        menuStateRender(&menu.menu);

		finishRenderFrame();

        if ((pad[0]->button & U_CBUTTONS) && (~lastButton & U_CBUTTONS))
        {
            paused = 0;
        }

        lastButton = pad[0]->button;
    }
}

void memoryAddressesRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    char strBuffer[5];
    int index;
    struct MemoryAddressMenuItem* address = (struct MemoryAddressMenuItem*)menuItem->data;

    FONTCOL(255, 255, 255, 255);
    
    for (index = 0; index < MEMORY_BLOCK_ROWS; ++index)
    {
        if (address->menuState->cursorY == index && menuItem == highlightedItem)
        {
            FONTCOL(43, 200, 100, 255);
        }
        else
        {
            FONTCOL(255, 255, 255, 255);
        }

        sprintf(strBuffer, "%04X", address->address[index]);
        SHOWFONT(&glistp, strBuffer, MEMROY_GRID_X, MEMORY_GRID_Y + index * DEBUG_MENU_ROW_HEIGHT);
    }
}

struct MenuItem* memoryAddressesHandleInput(struct MenuItem* menuItem, int buttons)
{
    struct MemoryAddressMenuItem* address = (struct MemoryAddressMenuItem*)menuItem->data;

    if ((buttons & U_JPAD) && address->menuState->cursorY > 0)
    {
        --address->menuState->cursorY;
        return menuItem;
    }
    else if ((buttons & D_JPAD) && address->menuState->cursorY < MEMORY_BLOCK_ROWS - 1)
    {
        ++address->menuState->cursorY;
        return menuItem;
    }

    return NULL;
}

void memoryValuesRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    char strBuffer[3];
    int x, y;
    struct MemoryValueMenuItem* values = (struct MemoryValueMenuItem*)menuItem->data;

    FONTCOL(255, 255, 255, 255);
    
    for (y = 0; y < MEMORY_BLOCK_ROWS; ++y)
    {
        for (x = 0; x < MEMORY_BLOCK_COLS; ++x)
        {
            if (values->menuState->cursorY == y && values->menuState->cursorX == x && menuItem == highlightedItem)
            {
                FONTCOL(43, 200, 100, 255);
            }
            else
            {
                FONTCOL(255, 255, 255, 255);
            }

            sprintf(strBuffer, "%02X", values->values[y * MEMORY_BLOCK_COLS + x]);
            SHOWFONT(&glistp, strBuffer, MEMORY_VALUES_X + x *  MEMORY_VALUES_SPACING, MEMORY_GRID_Y + y * DEBUG_MENU_ROW_HEIGHT);
        }
    }
}

struct MenuItem* memoryValuesHandleInput(struct MenuItem* menuItem, int buttons)
{
    struct MemoryValueMenuItem* values = (struct MemoryValueMenuItem*)menuItem->data;

    if ((buttons & U_JPAD) && values->menuState->cursorY > 0)
    {
        --values->menuState->cursorY;
        return menuItem;
    }
    else if ((buttons & D_JPAD) && values->menuState->cursorY < MEMORY_BLOCK_ROWS - 1)
    {
        ++values->menuState->cursorY;
        return menuItem;
    }
    else if ((buttons & L_JPAD) && values->menuState->cursorX > 0)
    {
        --values->menuState->cursorX;
        return menuItem;
    }
    else if ((buttons & R_JPAD) && values->menuState->cursorX < MEMORY_BLOCK_COLS - 1)
    {
        ++values->menuState->cursorX;
        return menuItem;
    }

    return NULL;
}

void cpuStateRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    struct CPUState* cpu = (struct CPUState*)menuItem->data;
    char buffer[8];
    FONTCOL(255, 255, 255, 255);

    sprintf(buffer, "AF %02X%02X", cpu->a, cpu->f);
    SHOWFONT(&glistp, buffer, CPU_STATE_X, CPU_STATE_Y + 0 * DEBUG_MENU_ROW_HEIGHT);
    sprintf(buffer, "BC %02X%02X", cpu->b, cpu->c);
    SHOWFONT(&glistp, buffer, CPU_STATE_X, CPU_STATE_Y + 1 * DEBUG_MENU_ROW_HEIGHT);
    sprintf(buffer, "DE %02X%02X", cpu->d, cpu->e);
    SHOWFONT(&glistp, buffer, CPU_STATE_X, CPU_STATE_Y + 2 * DEBUG_MENU_ROW_HEIGHT);
    sprintf(buffer, "HL %02X%02X", cpu->h, cpu->l);
    SHOWFONT(&glistp, buffer, CPU_STATE_X, CPU_STATE_Y + 3 * DEBUG_MENU_ROW_HEIGHT);
    
    sprintf(buffer, "SP %04X", cpu->sp);
    SHOWFONT(&glistp, buffer, CPU_STATE_X, CPU_STATE_Y + 4 * DEBUG_MENU_ROW_HEIGHT);
    sprintf(buffer, "PC %04X", cpu->pc);
    SHOWFONT(&glistp, buffer, CPU_STATE_X, CPU_STATE_Y + 5 * DEBUG_MENU_ROW_HEIGHT);
}

void initDebugMenu(struct DebuggerMenu* menu, struct CPUState* cpu, struct Memory* memory)
{
    zeroMemory(menu, sizeof(struct DebuggerMenu));

    menu->memoryAddresses.values = &menu->memoryValues;
    menu->memoryAddresses.menuState = &menu->state;
    menu->memoryAddresses.memory = memory;
    menu->memoryValues.menuState = &menu->state;
    menu->memoryValues.memory = memory;

    menuItemInit(
        &menu->menuItems[DebuggerMenuIndicesMemoryAddress],
        &menu->memoryAddresses,
        memoryAddressesRender,
        memoryAddressesHandleInput,
        NULL 
    );
    
    menuItemInit(
        &menu->menuItems[DebuggerMenuIndicesMemoryValues],
        &menu->memoryValues,
        memoryValuesRender,
        memoryValuesHandleInput,
        NULL 
    );
    
    menuItemInit(
        &menu->menuItems[DebuggerMenuIndicesCPUState],
        cpu,
        cpuStateRender,
        NULL,
        NULL 
    );

    menu->menuItems[DebuggerMenuIndicesMemoryAddress].toRight = &menu->menuItems[DebuggerMenuIndicesMemoryValues];
    menu->menuItems[DebuggerMenuIndicesMemoryValues].toLeft = &menu->menuItems[DebuggerMenuIndicesMemoryAddress];

    initMenuState(&menu->menu, menu->menuItems, 3);
}