
#ifndef _MENU_H
#define _MENU_H

#include <ultra64.h>

#define INITIAL_TIME_DELAY 30
#define REPEAT_TIME_DELAY 10

struct MenuItem;

typedef void (*MenuItemRender)(struct MenuItem* menuItem, struct MenuItem* highlightedItem);
typedef struct MenuItem* (*MenuItemHandleInput)(struct MenuItem* menuItem, int buttons);
typedef void (*MenuItemSetActive)(struct MenuItem* menuItem, int isActive);

struct MenuItem {
    struct MenuItem* toLeft;
    struct MenuItem* toRight;
    struct MenuItem* toUp;
    struct MenuItem* toDown;
    void* data;
    MenuItemRender renderCallback;
    MenuItemHandleInput handleButtonDown;
    MenuItemSetActive setActive;
};

struct MenuState {
    struct MenuItem* currentMenuItem;
    struct MenuItem* allItems;
    int menuItemCount;
    int lastButtons;
    int holdTimer;
};

void initMenuState(struct MenuState* menu, struct MenuItem* items, int itemCount);

void menuStateHandleInput(struct MenuState* menu, OSContPad* pad);
void menuStateRender(struct MenuState* menu);
void menuItemInit(struct MenuItem* menuItem, void* data, MenuItemRender renderCallback, MenuItemHandleInput handleButtonDown, MenuItemSetActive setActive);

#endif