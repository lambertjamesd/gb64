
#ifndef _ERASE_MENU_H
#define _ERASE_MENU_H

#include "menu.h"

enum EraseMenuItem {
    EraseMenuItemNo,
    EraseMenuItemYes,
    EraseMenuItemCount,
};

struct EraseMenu {
    struct CursorMenu cursor;
    struct CursorMenuItem menuItems[EraseMenuItemCount];
};

void initEraseMenu(struct EraseMenu* menu, struct MenuItem* parent);
void eraseMenuRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem);
struct MenuItem* eraseMenuHandleInput(struct MenuItem* menuItem, int buttonsDown, int buttonsState);

#endif