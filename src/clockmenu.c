
#include "clockmenu.h"
#include "gameboy.h"
#include "../render.h"
#include "debug_out.h"
#include "spritefont.h"
#include "sprite.h"

static char* hourLabels[] = {
    "12 AM",
    "1  AM",
    "2  AM",
    "3  AM",
    "4  AM",
    "5  AM",
    "6  AM",
    "7  AM",
    "8  AM",
    "9  AM",
    "10 AM",
    "11 AM",
    "12 PM",
    "1  PM",
    "2  PM",
    "3  PM",
    "4  PM",
    "5  PM",
    "6  PM",
    "7  PM",
    "8  PM",
    "9  PM",
    "10 PM",
    "11 PM",
};

void setClockMenuActive(struct MenuItem* menuItem, int isActive)
{
    struct ClockMenu* menu = (struct ClockMenu*)menuItem->data;

    if (isActive)
    {
        writeMBC3ClockRegisters(gGameboy.memory.misc.time, menu->registers);
        menu->day.cursorMenuItem.value = 
            ((menu->registers[REG_RTC_DH - REG_RTC_S] & 0x1) << 8) |
            menu->registers[REG_RTC_DL - REG_RTC_S];
        menu->hour.cursorMenuItem.value = menu->registers[REG_RTC_H - REG_RTC_S];
        menu->minute.cursorMenuItem.value = menu->registers[REG_RTC_M - REG_RTC_S];
        menu->second.cursorMenuItem.value = menu->registers[REG_RTC_S - REG_RTC_S];
    }
    else
    {
        menu->registers[REG_RTC_DH - REG_RTC_S] = (menu->day.cursorMenuItem.value >> 8) & 0x1;
        menu->registers[REG_RTC_DL - REG_RTC_S] = menu->day.cursorMenuItem.value & 0xFF;
        menu->registers[REG_RTC_H - REG_RTC_S] = menu->hour.cursorMenuItem.value;
        menu->registers[REG_RTC_M - REG_RTC_S] = menu->minute.cursorMenuItem.value;
        menu->registers[REG_RTC_S - REG_RTC_S] = menu->second.cursorMenuItem.value;

        gGameboy.memory.misc.time = readMBC3ClockRegisters(menu->registers);
        writeMBC3ClockRegisters(gGameboy.memory.misc.time, &READ_REGISTER_DIRECT(&gGameboy.memory, REG_RTC_S));
    }
}

void initClockMenu(struct ClockMenu* menu, struct MenuItem* parent)
{
    initCursorMenu(
        &menu->cursor,
        menu->menuItems,
        ClockMenuItemCount
    );
    menu->cursor.parentMenu = parent;

    initCursorMenuItem(
        &menu->menuItems[ClockMenuItemDay],
        NULL,
        "Day",
        32
    );
    initSelectCursorMenuItem(
        &menu->day.cursorMenuItem, 
        ClockMenuItemDay, 
        0, 
        0, 512, 
        NULL
    );
    menu->menuItems[ClockMenuItemDay].data = &menu->day;
    menu->menuItems[ClockMenuItemDay].render = renderSelectCursorMenuItem;
    menu->menuItems[ClockMenuItemDay].input = inputSelectCursorMenuItem;
    menu->day.registers = menu->registers;
    menu->day.registerIndex = REG_RTC_DL;

    initCursorMenuItem(
        &menu->menuItems[ClockMenuItemHour],
        NULL,
        "Hour",
        32
    );
    initSelectCursorMenuItem(
        &menu->hour.cursorMenuItem, 
        ClockMenuItemHour, 
        0, 
        0, 24, 
        hourLabels
    );
    menu->menuItems[ClockMenuItemHour].data = &menu->hour;
    menu->menuItems[ClockMenuItemHour].render = renderSelectCursorMenuItem;
    menu->menuItems[ClockMenuItemHour].input = inputSelectCursorMenuItem;
    menu->day.registers = menu->registers;
    menu->day.registerIndex = REG_RTC_H;

    initCursorMenuItem(
        &menu->menuItems[ClockMenuItemMinute],
        NULL,
        "Minute",
        32
    );
    initSelectCursorMenuItem(
        &menu->minute.cursorMenuItem, 
        ClockMenuItemMinute, 
        0, 
        0, 60, 
        NULL
    );
    menu->menuItems[ClockMenuItemMinute].data = &menu->minute;
    menu->menuItems[ClockMenuItemMinute].render = renderSelectCursorMenuItem;
    menu->menuItems[ClockMenuItemMinute].input = inputSelectCursorMenuItem;
    menu->day.registers = menu->registers;
    menu->day.registerIndex = REG_RTC_M;

    initCursorMenuItem(
        &menu->menuItems[ClockMenuItemSecond],
        NULL,
        "Second",
        32
    );
    initSelectCursorMenuItem(
        &menu->second.cursorMenuItem, 
        ClockMenuItemSecond, 
        0, 
        0, 60, 
        NULL
    );
    menu->menuItems[ClockMenuItemSecond].data = &menu->second;
    menu->menuItems[ClockMenuItemSecond].render = renderSelectCursorMenuItem;
    menu->menuItems[ClockMenuItemSecond].input = inputSelectCursorMenuItem;
    menu->day.registers = menu->registers;
    menu->day.registerIndex = REG_RTC_S;
}

void clockMenuRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    struct ClockMenu* clockMenu = (struct ClockMenu*)menuItem->data;
    
    if (menuItem == highlightedItem)
    {
        renderMenuBorder();
        spriteSetColor(gGBFont.spriteLayer, 255, 255, 255, 255);
        renderText(&gGBFont, "CLOCK", 16, 32, 0);
        renderCursorMenu(&clockMenu->cursor, 16, 56, 136);
    }
}

struct MenuItem* clockMenuHandleInput(struct MenuItem* menuItem, int buttonsDown, int buttonsState)
{
    struct ClockMenu* clockMenu = (struct ClockMenu*)menuItem->data;
    if (buttonsDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.openMenu))
    {
        return clockMenu->cursor.parentMenu;
    }

    struct MenuItem* result = inputCursorMenu(&clockMenu->cursor, buttonsDown, 136);

    if (result)
    {
        return result;
    }
    else
    {
        return menuItem;
    }
}