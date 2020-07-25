
#ifndef _CLOCK_MENU_H
#define _CLOCK_MENU_H

#include "menu.h"

enum ClockMenuItem {
    ClockMenuItemDay,
    ClockMenuItemHour,
    ClockMenuItemMinute,
    ClockMenuItemSecond,
    ClockMenuItemCount,
};

struct ClockCursorMenuItem {
    struct SelectCursorMenuItem cursorMenuItem;
    u8* registers;
    int registerIndex;
};

struct ClockMenu {
    struct CursorMenu cursor;
    struct CursorMenuItem menuItems[ClockMenuItemCount];
    struct ClockCursorMenuItem day;
    struct ClockCursorMenuItem hour;
    struct ClockCursorMenuItem minute;
    struct ClockCursorMenuItem second;
    u8 registers[8];
};

void initClockMenu(struct ClockMenu* menu, struct MenuItem* parent);
void clockMenuRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem);
struct MenuItem* clockMenuHandleInput(struct MenuItem* menuItem, int buttonsDown, int buttonsState);
void setClockMenuActive(struct MenuItem* menuItem, int isActive);

#endif