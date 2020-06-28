#include <ultra64.h>
#include "debugger.h"
#include "debug_out.h"
#include "../memory.h"
#include "../render.h"
#include "decoder.h"
#include "bool.h"

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

struct Breakpoint* findBreakpoint(struct Memory* memory, u8* at)
{
    int index;

    for (index = 0; index < BREAK_POINT_COUNT; ++index)
    {
        if (memory->breakpoints[index].memoryAddress == at)
        {
            return &memory->breakpoints[index];
        }
    }

    return NULL;
}

void insertBreakpoint(struct Memory* memory, u16 address, enum BreakpointType type, enum BreakpointType breakpointIndex)
{
    u8* memoryAddress = getMemoryAddress(memory, address);
    struct Breakpoint* existingBreakpoint = findBreakpoint(memory, memoryAddress);

    if (existingBreakpoint)
    {
        // don't add a breakpoint if one already exists
        return;
    }

    if (memory->breakpoints[breakpointIndex].breakpointType != BreakpointTypeNone)
    {
        removeBreakpointDirect(&memory->breakpoints[breakpointIndex]);
    }

    memory->breakpoints[breakpointIndex].memoryAddress = memoryAddress;
    memory->breakpoints[breakpointIndex].address = address;
    memory->breakpoints[breakpointIndex].existingInstruction = *memory->breakpoints[breakpointIndex].memoryAddress;
    memory->breakpoints[breakpointIndex].breakpointType = type;

    *memory->breakpoints[breakpointIndex].memoryAddress = DEBUG_INSTRUCTION;
}

struct Breakpoint* addBreakpoint(struct Memory* memory, u16 address, enum BreakpointType type)
{
    int index;
    for (index = 0; index < USER_BREAK_POINTS; ++index)
    {
        if (memory->breakpoints[index].breakpointType == BreakpointTypeNone)
        {
            insertBreakpoint(memory, address, BreakpointTypeUser, index);
            return &memory->breakpoints[index];
        }
    }

    return NULL;
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

void reapplyBreakpoint(struct Breakpoint* breakpoint)
{
    if (breakpoint->breakpointType != BreakpointTypeNone && *breakpoint->memoryAddress != DEBUG_INSTRUCTION)
    {
        breakpoint->existingInstruction = *breakpoint->memoryAddress;
        *breakpoint->memoryAddress = DEBUG_INSTRUCTION;
    }
}

struct AddressTuple getInstructionBranchFromState(struct CPUState* cpu, struct Memory* memory, u8 instructionCode)
{
    u8 instructionData[4];
    instructionData[0] = instructionCode;
    instructionData[1] = readMemoryDirect(memory, cpu->pc + 1);
    instructionData[2] = readMemoryDirect(memory, cpu->pc + 2);
    return getInstructionBranch(
        instructionData,
        cpu->pc,
        (u16)readMemoryDirect(memory, cpu->sp) | ((u16)readMemoryDirect(memory, cpu->sp + 1) << 8),
        ((u16)cpu->h << 8) | cpu->l
    );
}

///////////////////////////////////

void editValueRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    if (menuItem == highlightedItem)
    {
        int index;
        char strBuffer[2];
        struct EditValueMenuState* editMenu = (struct EditValueMenuState*)menuItem->data;
        FONTCOL(255, 255, 255, 255);

        for (index = 0; index < editMenu->nibbleWidth; ++index)
        {
            if (editMenu->currentNibble == index)
            {
                FONTCOL(200, 100, 45, 255);
            }
            else
            {
                FONTCOL(45, 100, 255, 255);
            }

            sprintf(strBuffer, "%1X", (editMenu->currentValue >> (index * 4)) & 0xF);
            SHOWFONT(&glistp, strBuffer, editMenu->x + DEBUGGER_FONT_W * (editMenu->nibbleWidth - index - 1), editMenu->y);
        }
    }
}

struct MenuItem* editValueHandleInput(struct MenuItem* menuItem, int buttons, int buttonsState)
{
    struct EditValueMenuState* editMenu = (struct EditValueMenuState*)menuItem->data;
    u16 offset = 0x1 << (editMenu->currentNibble * 4);
    u16 mask = 0xF << (editMenu->currentNibble * 4);
    
    if (buttons & B_BUTTON)
    {
        editMenu->confirmEdit = 0;
        return editMenu->returnTo;
    }
    else if (buttons & (A_BUTTON | START_BUTTON))
    {
        editMenu->confirmEdit = 1;
        return editMenu->returnTo;
    }
    else if (buttons & U_JPAD)
    {
        editMenu->currentValue = (editMenu->currentValue & ~mask) | ((editMenu->currentValue + offset) & mask);
    }
    else if (buttons & D_JPAD)
    {
        editMenu->currentValue = (editMenu->currentValue & ~mask) | ((editMenu->currentValue - offset) & mask);
    }
    else if ((buttons & R_JPAD) && editMenu->currentNibble > 0)
    {
        --editMenu->currentNibble;
    }
    else if ((buttons * L_JPAD) && editMenu->currentNibble < editMenu->nibbleWidth - 1)
    {
        ++editMenu->currentNibble;
    }

    return menuItem;
}

struct MenuItem* startEdit(struct MenuItem* editMenuItem, struct MenuItem* returnTo, u16 x, u16 y, u16 nibbleWidth, u16 initialValue)
{
    struct EditValueMenuState* editMenu = (struct EditValueMenuState*)editMenuItem->data;

    editMenu->returnTo = returnTo;
    editMenu->x = x;
    editMenu->y = y;
    editMenu->nibbleWidth = nibbleWidth;
    editMenu->currentNibble = 0;
    editMenu->currentValue = initialValue;
    editMenu->confirmEdit = 0;

    return editMenuItem;
}

bool checkEditValue(struct MenuItem* editMenuItem, struct MenuItem* targetMenu, u16* value)
{
    struct EditValueMenuState* editMenu = (struct EditValueMenuState*)editMenuItem->data;

    if (editMenu->returnTo == targetMenu)
    {
        editMenu->returnTo = NULL;
        *value = editMenu->currentValue;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

///////////////////////////////////

void memoryAddressesRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    char strBuffer[5];
    int index;
    struct MemoryAddressMenuItem* addressGUI = (struct MemoryAddressMenuItem*)menuItem->data;
    struct EditValueMenuState* editMenu = (struct EditValueMenuState*)addressGUI->editMenuItem->data;
    u16 address = addressGUI->addressStart;

    FONTCOL(255, 255, 255, 255);
    
    for (index = 0; index < MEMORY_BLOCK_ROWS; ++index)
    {
        if (editMenu->returnTo != menuItem || addressGUI->menuState->cursorY != index)
        {
            if (addressGUI->menuState->cursorY == index && menuItem == highlightedItem)
            {
                FONTCOL(43, 200, 100, 255);
            }
            else
            {
                FONTCOL(255, 255, 255, 255);
            }

            sprintf(strBuffer, "%04X", address);
            SHOWFONT(&glistp, strBuffer, MEMORY_GRID_X, MEMORY_GRID_Y + index * DEBUG_MENU_ROW_HEIGHT);
        }
        address += MEMORY_BLOCK_COLS;
    }
}

struct MenuItem* memoryAddressesHandleInput(struct MenuItem* menuItem, int buttons, int buttonsState)
{
    struct MemoryAddressMenuItem* address = (struct MemoryAddressMenuItem*)menuItem->data;

    if (buttons & U_JPAD)
    {
        if (address->menuState->cursorY > 0) 
        {
            --address->menuState->cursorY;
        } 
        else 
        {
            address->addressStart -= MEMORY_BLOCK_COLS;
        }
        return menuItem;
    }
    else if (buttons & D_JPAD)
    {
        if ( address->menuState->cursorY < MEMORY_BLOCK_ROWS - 1)
        {
            ++address->menuState->cursorY;
        }
        else
        {
            address->addressStart += MEMORY_BLOCK_COLS;
        }
        return menuItem;
    }
    else if (buttons & (A_BUTTON | START_BUTTON))
    {
        return startEdit(
            address->editMenuItem, 
            menuItem,
            MEMORY_GRID_X,
            MEMORY_GRID_Y + address->menuState->cursorY * DEBUG_MENU_ROW_HEIGHT,
            4,
            address->addressStart + address->menuState->cursorY * MEMORY_BLOCK_COLS
        );
    }

    return NULL;
}

void memoryAddressSetActive(struct MenuItem* menuItem, int isActive)
{
    struct MemoryAddressMenuItem* address = (struct MemoryAddressMenuItem*)menuItem->data;

    if (isActive)
    {
        u16 newValue;

        if (checkEditValue(address->editMenuItem, menuItem, &newValue))
        {
            address->addressStart = newValue - address->menuState->cursorY * MEMORY_BLOCK_COLS;
        }
    }
}

///////////////////////////////////

void memoryValuesCheckReread(struct MemoryValueMenuItem* values)
{
    if (values->addressStart != values->addressGUI->addressStart)
    {
        values->addressStart = values->addressGUI->addressStart;
        int index;
        for (index = 0; index < MEMORY_BLOCK_ROWS * MEMORY_BLOCK_COLS; ++index)
        {
            values->values[index] = readMemoryDirect(values->memory, values->addressStart + index);
        }
    }
}

void memoryValuesRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    char strBuffer[3];
    int x, y;
    struct MemoryValueMenuItem* values = (struct MemoryValueMenuItem*)menuItem->data;
    struct EditValueMenuState* editMenu = (struct EditValueMenuState*)values->editMenuItem->data;
    memoryValuesCheckReread(values);

    FONTCOL(255, 255, 255, 255);
    
    for (y = 0; y < MEMORY_BLOCK_ROWS; ++y)
    {
        for (x = 0; x < MEMORY_BLOCK_COLS; ++x)
        {
            if (editMenu->returnTo == menuItem && values->menuState->cursorY == y && values->menuState->cursorX == x)
                continue;

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

struct MenuItem* memoryValuesHandleInput(struct MenuItem* menuItem, int buttons, int buttonsState)
{
    struct MemoryValueMenuItem* values = (struct MemoryValueMenuItem*)menuItem->data;

    if (buttons & U_JPAD)
    {
        if (values->menuState->cursorY > 0) 
        {
            --values->menuState->cursorY;
        } 
        else 
        {
            values->addressGUI->addressStart -= MEMORY_BLOCK_COLS;
        }
        return menuItem;
    }
    else if (buttons & D_JPAD)
    {
        if (values->menuState->cursorY < MEMORY_BLOCK_ROWS - 1)
        {
            ++values->menuState->cursorY;
        }
        else
        {
            values->addressGUI->addressStart += MEMORY_BLOCK_COLS;
        }
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
    else if (buttons & (A_BUTTON | START_BUTTON))
    {
        return startEdit(
            values->editMenuItem, 
            menuItem,
            MEMORY_VALUES_X + values->menuState->cursorX * MEMORY_VALUES_SPACING,
            MEMORY_GRID_Y + values->menuState->cursorY * DEBUG_MENU_ROW_HEIGHT,
            2,
            values->values[values->menuState->cursorX + values->menuState->cursorY * MEMORY_BLOCK_COLS]
        );
    }

    return NULL;
}

void memoryValuesSetActive(struct MenuItem* menuItem, int isActive)
{
    struct MemoryValueMenuItem* values = (struct MemoryValueMenuItem*)menuItem->data;

    if (isActive)
    {
        u16 newValue;

        if (checkEditValue(values->editMenuItem, menuItem, &newValue))
        {
            u16 offset = values->menuState->cursorX + values->menuState->cursorY * MEMORY_BLOCK_COLS;
            values->values[offset] = newValue;
            writeMemoryDirect(values->memory, values->addressStart + offset, newValue);
        }
    }
}

///////////////////////////////////

void instructionsReload(struct InstructionsMenuItem* instructions, u16 currentAddress)
{
    int index;
    instructions->startAddress = currentAddress;

    for (index = 0; index < MI_INSTRUCTIONS_LINE_COUNT; ++index)
    {
        u8 instruction = readMemoryDirect(instructions->memory, currentAddress);

        if (instruction == DEBUG_INSTRUCTION)
        {
            instructions->instructions[index].breakpoint = findBreakpoint(instructions->memory, getMemoryAddress(instructions->memory, currentAddress));

            if (instructions->instructions[index].breakpoint)
            {
                instruction = instructions->instructions[index].breakpoint->existingInstruction;
            }
        }
        else
        {
            instructions->instructions[index].breakpoint = NULL;
        }

        instructions->instructions[index].address = currentAddress;
        int instructionSize = getInstructionSize(instruction);

        switch (getInstructionSize(instruction))
        {
            case 3:
                instructions->instructions[index].instruction[2] = readMemoryDirect(instructions->memory, currentAddress + 2);
            case 2:
                instructions->instructions[index].instruction[1] = readMemoryDirect(instructions->memory, currentAddress + 1);
            default:
                instructions->instructions[index].instruction[0] = instruction;
        }

        currentAddress = currentAddress + instructionSize;
    }
}

void instructionsRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    int index;
    char strBuffer[16];
    struct InstructionsMenuItem* instructions = (struct InstructionsMenuItem*)menuItem->data;
    struct EditValueMenuState* editMenu = (struct EditValueMenuState*)instructions->editMenuItem->data;
    FONTCOL(255, 255, 255, 255);

    for (index = 0; index < MI_INSTRUCTIONS_LINE_COUNT; ++index)
    {
        struct InstructionLine* line = &instructions->instructions[index];

        if (editMenu->returnTo != menuItem || instructions->menuState->cursorY != index)
        {
            if (instructions->menuState->cursorY == index && menuItem == highlightedItem && instructions->menuState->cursorX == MI_INSTRUCTION_ADDR_COL)
            {
                FONTCOL(43, 200, 100, 255);
            }
            else
            {
                FONTCOL(255, 255, 255, 255);
            }
            sprintf(strBuffer, "%04X", line->address);
            SHOWFONT(&glistp, strBuffer, MI_INSTRUCTIONS_X, MI_INSTRUCTIONS_Y + index * DEBUG_MENU_ROW_HEIGHT);
        }

        decodeInstruction(strBuffer, line->instruction, line->address);

        if (instructions->menuState->cursorY == index && menuItem == highlightedItem && instructions->menuState->cursorX == MI_INSTRUCTION_NAME_COL)
        {
            if (line->breakpoint)
            {
                FONTCOL(200, 200, 43, 255);
            }
            else
            {
                FONTCOL(43, 200, 100, 255);
            }
        }
        else
        {
            if (line->breakpoint)
            {
                FONTCOL(200, 100, 43, 255);
            }
            else
            {
                FONTCOL(255, 255, 255, 255);
            }
        }

        SHOWFONT(&glistp, strBuffer, MI_INSTRUCTIONS_NAME_X, MI_INSTRUCTIONS_Y + index * DEBUG_MENU_ROW_HEIGHT);

        if (line->address == instructions->cpu->pc)
        {
            FONTCOL(200, 100, 43, 255);
            SHOWFONT(&glistp, "-", MI_INSTRUCTIONS_X - DEBUGGER_FONT_W, MI_INSTRUCTIONS_Y + index * DEBUG_MENU_ROW_HEIGHT);
        }
    }
}

struct MenuItem* instructionsHandleInput(struct MenuItem* menuItem, int buttons, int buttonsState)
{
    struct InstructionsMenuItem* instructions = (struct InstructionsMenuItem*)menuItem->data;
    
    if (buttons & U_JPAD)
    {
        if (instructions->menuState->cursorY > 0) 
        {
            --instructions->menuState->cursorY;
        } 
        // TODO scroll instructions
        return menuItem;
    }
    else if (buttons & D_JPAD)
    {
        if (instructions->menuState->cursorY < MI_INSTRUCTIONS_LINE_COUNT - 1)
        {
            ++instructions->menuState->cursorY;
        }
        else
        {
            instructionsReload(instructions, instructions->instructions[1].address);
        }
        return menuItem;
    }
    else if ((buttons & L_JPAD) && instructions->menuState->cursorX == MI_INSTRUCTION_NAME_COL)
    {
        --instructions->menuState->cursorX;
        return menuItem;
    }
    else if ((buttons & R_JPAD) && instructions->menuState->cursorX == MI_INSTRUCTION_ADDR_COL)
    {
        ++instructions->menuState->cursorX;
        return menuItem;
    }
    else if (buttons & (A_BUTTON | START_BUTTON))
    {
        if (instructions->menuState->cursorX == MI_INSTRUCTION_ADDR_COL)
        {
            return startEdit(
                instructions->editMenuItem, 
                menuItem,
                MI_INSTRUCTIONS_X,
                MI_INSTRUCTIONS_Y + instructions->menuState->cursorY * DEBUG_MENU_ROW_HEIGHT,
                4,
                instructions->instructions[instructions->menuState->cursorY].address
            );
        }
        else
        {   
            if (instructions->menuState->cursorY >= 0 && instructions->menuState->cursorY < MI_INSTRUCTIONS_LINE_COUNT)
            {
                struct InstructionLine* line = &instructions->instructions[instructions->menuState->cursorY];

                if (line->breakpoint)
                {
                    removeBreakpointDirect(line->breakpoint);
                    line->breakpoint = NULL;
                }
                else
                {
                    line->breakpoint = addBreakpoint(instructions->memory, instructions->instructions[instructions->menuState->cursorY].address, BreakpointTypeUser);
                }
            }

            return menuItem;
        }
    }
    else if (buttons & Z_TRIG)
    {
        insertBreakpoint(instructions->memory, instructions->instructions[instructions->menuState->cursorY].address, BreakpointTypeStep, SYSTEM_BREAK_POINT_START);
        instructions->menuState->isDebugging = FALSE;
    }

    return NULL;
}

void instructionsSetActive(struct MenuItem* menuItem, int isActive)
{
    struct InstructionsMenuItem* instructions = (struct InstructionsMenuItem*)menuItem->data;

    if (isActive)
    {
        u16 newValue;

        if (checkEditValue(instructions->editMenuItem, menuItem, &newValue))
        {
            instructionsReload(instructions, newValue);
        }
    }
}

///////////////////////////////////

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
    
    sprintf(buffer, "EI %1X", cpu->interrupts);
    SHOWFONT(&glistp, buffer, CPU_STATE_X + 8 * DEBUGGER_FONT_W, CPU_STATE_Y + 0 * DEBUG_MENU_ROW_HEIGHT);

    sprintf(buffer, "TK %03X", cpu->cyclesRun & 0xFFFFFF);
    SHOWFONT(&glistp, buffer, CPU_STATE_X + 8 * DEBUGGER_FONT_W, CPU_STATE_Y + 1 * DEBUG_MENU_ROW_HEIGHT);
}

///////////////////////////////////

void initDebugMenu(struct DebuggerMenu* menu, struct CPUState* cpu, struct Memory* memory)
{
    zeroMemory(menu, sizeof(struct DebuggerMenu));

    menu->state.isDebugging = TRUE;
    menu->state.cursorX = MI_INSTRUCTION_NAME_COL;
    menu->memoryAddresses.menuState = &menu->state;
    menu->memoryAddresses.editMenuItem = &menu->menuItems[DebuggerMenuIndicesEditValue];

    menu->memoryValues.menuState = &menu->state;
    menu->memoryValues.memory = memory;
    menu->memoryValues.addressGUI = &menu->memoryAddresses;
    menu->memoryValues.addressStart = 1; // this forces a page refresh
    menu->memoryValues.editMenuItem = &menu->menuItems[DebuggerMenuIndicesEditValue];

    menu->instructionMenuItem.menuState = &menu->state;
    menu->instructionMenuItem.cpu = cpu;
    menu->instructionMenuItem.memory = memory;
    menu->instructionMenuItem.editMenuItem = &menu->menuItems[DebuggerMenuIndicesEditValue];
    instructionsReload(&menu->instructionMenuItem, cpu->pc);

    menuItemInit(
        &menu->menuItems[DebuggerMenuIndicesMemoryAddress],
        &menu->memoryAddresses,
        memoryAddressesRender,
        memoryAddressesHandleInput,
        memoryAddressSetActive 
    );
    
    menuItemInit(
        &menu->menuItems[DebuggerMenuIndicesMemoryValues],
        &menu->memoryValues,
        memoryValuesRender,
        memoryValuesHandleInput,
        memoryValuesSetActive 
    );
    
    menuItemInit(
        &menu->menuItems[DebuggerMenuIndicesCPUState],
        cpu,
        cpuStateRender,
        NULL,
        NULL 
    );

    menuItemInit(
        &menu->menuItems[DebuggerMenuIndicesInstructions],
        &menu->instructionMenuItem,
        instructionsRender,
        instructionsHandleInput,
        instructionsSetActive 
    );

    menuItemInit(
        &menu->menuItems[DebuggerMenuIndicesEditValue],
        &menu->editMenu,
        editValueRender,
        editValueHandleInput,
        NULL 
    );

    menuItemConnectSideToSide(&menu->menuItems[DebuggerMenuIndicesMemoryAddress], &menu->menuItems[DebuggerMenuIndicesMemoryValues]);
    menuItemConnectSideToSide(&menu->menuItems[DebuggerMenuIndicesMemoryValues], &menu->menuItems[DebuggerMenuIndicesInstructions]);

    initMenuState(&menu->menu, menu->menuItems, DebuggerMenuIndicesCount);
}

struct DebuggerMenu gDebugMenu;

u8 useDebugger(struct CPUState* cpu, struct Memory* memory)
{
    OSContPad	**pad;
    int lastButton = ~0;
    int index;

    u8 result = readMemoryDirect(memory, cpu->pc);

    if (result == DEBUG_INSTRUCTION)
    {
        struct Breakpoint* breakpoint = findBreakpoint(memory, getMemoryAddress(memory, cpu->pc));

        if (breakpoint)
        {
            result = breakpoint->existingInstruction;
        }
    }

    for (index = USER_BREAK_POINTS; index < BREAK_POINT_COUNT; ++index)
    {
        if (memory->breakpoints[index].breakpointType == BreakpointTypeStep)
        {
            removeBreakpointDirect(&memory->breakpoints[index]);
        }
    }
    
    gDebugMenu.memoryValues.addressStart = ~gDebugMenu.memoryAddresses.addressStart; // this forces a page refresh
    memoryValuesCheckReread(&gDebugMenu.memoryValues);
    if (cpu->pc < gDebugMenu.instructionMenuItem.instructions[0].address || cpu->pc >= gDebugMenu.instructionMenuItem.instructions[MI_INSTRUCTIONS_LINE_COUNT - 4].address) {
        instructionsReload(&gDebugMenu.instructionMenuItem, cpu->pc);
    }
    gDebugMenu.state.isDebugging = TRUE;

    while (gDebugMenu.state.isDebugging)
    {
		pad = ReadController(0);

        menuStateHandleInput(&gDebugMenu.menu, pad[0]);
    
		preRenderFrame();
		renderDebugLog();

        menuStateRender(&gDebugMenu.menu);

		finishRenderFrame();

        int buttonDown = pad[0]->button & ~lastButton;

        if (buttonDown & L_CBUTTONS)
        {
            gDebugMenu.state.isDebugging = FALSE;
        }
        else if (buttonDown & D_CBUTTONS)
        {
            struct AddressTuple possibleNext = getInstructionBranchFromState(cpu, memory, result);
            insertBreakpoint(memory, possibleNext.nextInstruction, BreakpointTypeStep, SYSTEM_BREAK_POINT_START);
            if (!isInstructionCall(result) && possibleNext.nextInstruction != possibleNext.branchInstruction)
            {
                insertBreakpoint(memory, possibleNext.branchInstruction, BreakpointTypeStep, SYSTEM_BREAK_POINT_START + 1);
            }
            gDebugMenu.state.isDebugging = FALSE;
        }
        else if (buttonDown & R_CBUTTONS)
        {
            struct AddressTuple possibleNext = getInstructionBranchFromState(cpu, memory, result);
            insertBreakpoint(memory, possibleNext.nextInstruction, BreakpointTypeStep, SYSTEM_BREAK_POINT_START);
            if (possibleNext.nextInstruction != possibleNext.branchInstruction)
            {
                insertBreakpoint(memory, possibleNext.branchInstruction, BreakpointTypeStep, SYSTEM_BREAK_POINT_START + 1);
            }
            gDebugMenu.state.isDebugging = FALSE;
        }

        lastButton = pad[0]->button;
    }

    clearDebugOutput();
    zeroMemory(cfb, sizeof(u16) * 2 * SCREEN_WD * SCREEN_HT);

    return result;
}