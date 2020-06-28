
#include "graphicsmenu.h"
#include "../render.h"
#include "gameboy.h"

void renderPalleteItem(struct CursorMenuItem* menuItem, int x, int y, int selected)
{
    FONTCOL(255, 255, 255, 255);
    SHOWFONT(&glistp, menuItem->label, x, y);

    u16* palleteIndexPtr = (u16*)menuItem->data;
    u16* pallete = getPallete(*palleteIndexPtr);

    if (selected)
    {
        renderSprite(&gGUIItemTemplates[GUIItemIconLeft], x, y + 12, 1, 1);
        renderSprite(&gGUIItemTemplates[GUIItemIconRight], x + 60, y + 12, 1, 1);
    }

    setSpriteColor(GET_R(pallete[0]), GET_G(pallete[0]), GET_B(pallete[0]));
    renderSprite(&gGUIItemTemplates[GUIItemIconWhite], x + 12, y + 12, 1, 1);
    setSpriteColor(GET_R(pallete[1]), GET_G(pallete[1]), GET_B(pallete[1]));
    renderSprite(&gGUIItemTemplates[GUIItemIconWhite], x + 24, y + 12, 1, 1);
    setSpriteColor(GET_R(pallete[2]), GET_G(pallete[2]), GET_B(pallete[2]));
    renderSprite(&gGUIItemTemplates[GUIItemIconWhite], x + 36, y + 12, 1, 1);
    setSpriteColor(GET_R(pallete[3]), GET_G(pallete[3]), GET_B(pallete[3]));
    renderSprite(&gGUIItemTemplates[GUIItemIconWhite], x + 48, y + 12, 1, 1);


    setSpriteColor(255, 255, 255);
}

struct MenuItem* inputPalleteItem(struct CursorMenuItem* menuItem, int buttonDown)
{
    u16* palleteIndexPtr = (u16*)menuItem->data;

    if (buttonDown & R_JPAD)
    {
        *palleteIndexPtr = (*palleteIndexPtr + 1) % getPalleteCount();
        updatePalleteInfo(&gGameboy);
    }

    if (buttonDown & L_JPAD)
    {
        if (*palleteIndexPtr == 0)
        {
            *palleteIndexPtr = getPalleteCount() - 1;
        }
        else
        {
            *palleteIndexPtr = *palleteIndexPtr - 1;
        }
        updatePalleteInfo(&gGameboy);
    }

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
        32
    );
    menu->menuItems[GraphicsMenuItemGBP].data = &gGameboy.settings.bgpIndex;
    menu->menuItems[GraphicsMenuItemGBP].render = renderPalleteItem;
    menu->menuItems[GraphicsMenuItemGBP].input = inputPalleteItem;
    
    initCursorMenuItem(
        &menu->menuItems[GraphicsMenuItemOBP0],
        NULL,
        "OBP0",
        32
    );
    menu->menuItems[GraphicsMenuItemOBP0].data = &gGameboy.settings.obp0Index;
    menu->menuItems[GraphicsMenuItemOBP0].render = renderPalleteItem;
    menu->menuItems[GraphicsMenuItemOBP0].input = inputPalleteItem;

    initCursorMenuItem(
        &menu->menuItems[GraphicsMenuItemOBP1],
        NULL,
        "OBP1",
        32
    );
    menu->menuItems[GraphicsMenuItemOBP1].data = &gGameboy.settings.obp1Index;
    menu->menuItems[GraphicsMenuItemOBP1].render = renderPalleteItem;
    menu->menuItems[GraphicsMenuItemOBP1].input = inputPalleteItem;
}