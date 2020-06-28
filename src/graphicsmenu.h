
#ifndef _GRAPHICS_MENU_H
#define _GRAPHICS_MENU_H

#include "menu.h"

enum GraphicsMenuItem {
    GraphicsMenuItemGBP,
    GraphicsMenuItemOBP0,
    GraphicsMenuItemOBP1,
    GraphicsMenuItemCount,
};

struct PalleteSelection {
    u16* targetIndex;
};

struct GraphicsMenu {
    struct CursorMenu cursor;
    struct CursorMenuItem menuItems[GraphicsMenuItemCount];
};

void initGraphicsMenu(struct GraphicsMenu* menu, struct MenuItem* parentMenu);
void graphicsMenuRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem);
struct MenuItem* graphicsMenuHandleInput(struct MenuItem* menuItem, int buttonsDown, int buttonsState);

#endif