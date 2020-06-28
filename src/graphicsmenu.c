
#include "graphicsmenu.h"
#include "../render.h"
#include "gameboy.h"

void renderPalleteItem(struct CursorMenuItem* menuItem, int x, int y, int selected)
{
    FONTCOL(255, 255, 255, 255);
    SHOWFONT(&glistp, menuItem->label, x, y);
}

struct MenuItem* inputPalleteItem(struct CursorMenuItem* menuItem, int buttonDown)
{
    return NULL;
}

void graphicsMenuRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    struct GraphicsMenu* graphicsMenu = (struct GraphicsMenu*)menuItem->data;
    
    if (menuItem == highlightedItem)
    {
        gButtonSprite.alpha = 255;
        renderMenuBorder();
        FONTCOL(255, 255, 255, 255);
        SHOWFONT(&glistp, "SCREEN", 8, 32);
        renderCursorMenu(&graphicsMenu->cursor, 8, 56, 136);
    }
}

struct MenuItem* graphicsMenuHandleInput(struct MenuItem* menuItem, int buttonsDown, int buttonsState)
{
    struct GraphicsMenu* graphicsMenu = (struct GraphicsMenu*)menuItem->data;
    if (buttonsDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.openMenu))
    {
        return graphicsMenu->cursor.parentMenu;
    }

    struct MenuItem* result = inputCursorMenu(&graphicsMenu->cursor, buttonsDown, 136);

    if (result)
    {
        return result;
    }
    else
    {
        return menuItem;
    }
}

void initGraphicsMenu(struct GraphicsMenu* menu, struct MenuItem* parentMenu)
{
    initCursorMenu(
        &menu->cursor,
        menu->menuItems,
        GraphicsMenuItemCount
    );
    menu->cursor.parentMenu = parentMenu;

    initCursorMenuItem(
        &menu->menuItems[GraphicsMenuItemGBP],
        NULL,
        "BGP",
        20
    );
    menu->menuItems[GraphicsMenuItemGBP].data = &gGameboy.settings.bgpIndex;
    menu->menuItems[GraphicsMenuItemGBP].render = renderPalleteItem;
    menu->menuItems[GraphicsMenuItemGBP].input = inputPalleteItem;
    
    initCursorMenuItem(
        &menu->menuItems[GraphicsMenuItemOBP0],
        NULL,
        "OBP0",
        20
    );
    menu->menuItems[GraphicsMenuItemOBP0].data = &gGameboy.settings.obp0Index;
    menu->menuItems[GraphicsMenuItemOBP0].render = renderPalleteItem;
    menu->menuItems[GraphicsMenuItemOBP0].input = inputPalleteItem;

    initCursorMenuItem(
        &menu->menuItems[GraphicsMenuItemOBP1],
        NULL,
        "OBP1",
        20
    );
    menu->menuItems[GraphicsMenuItemOBP1].data = &gGameboy.settings.obp1Index;
    menu->menuItems[GraphicsMenuItemOBP1].render = renderPalleteItem;
    menu->menuItems[GraphicsMenuItemOBP1].input = inputPalleteItem;
}