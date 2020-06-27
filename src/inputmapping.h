
#ifndef _INPUT_MAPPING_H
#define _INPUT_MAPPING_H

#include "menu.h"

#define MAPPED_INPUT_COUNT   11

struct InputMappingMenu {
    struct CursorMenu cursor;
    struct CursorMenuItem menuItems[MAPPED_INPUT_COUNT];
    int currentMappingIndex;
};

void initInputMappingMenu(struct InputMappingMenu* menu, struct MenuItem* parentMenu);
void inputMappingRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem);
struct MenuItem* inputMappingHandleInput(struct MenuItem* menuItem, int buttonsDown, int buttonsState);

#endif