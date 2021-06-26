
#include "erasemenu.h"
#include "spritefont.h"
#include "save.h"

void initEraseMenu(struct EraseMenu* menu, struct MenuItem* parent)
{
    initCursorMenu(&menu->cursor, menu->menuItems, EraseMenuItemCount);
    menu->cursor.parentMenu = parent;

    initCursorMenuItem(
        &menu->menuItems[EraseMenuItemNo],
        parent,
        "Cancel",
        32
    );

    initCursorMenuItem(
        &menu->menuItems[EraseMenuItemYes],
        parent,
        "Erase",
        32
    );
}

void eraseMenuRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    struct EraseMenu* eraseMenu = (struct EraseMenu*)menuItem->data;

    if (menuItem == highlightedItem)
    {
        renderMenuBorder();
        spriteSetColor(gGBFont.spriteLayer, 255, 255, 255, 255);
        renderText(&gGBFont, "ERASE", 32, 64, 1);
        renderCursorMenu(&eraseMenu->cursor, 32, 112, 272);
    }
}

struct MenuItem* eraseMenuHandleInput(struct MenuItem* menuItem, int buttonsDown, int buttonsState)
{
    struct EraseMenu* eraseMenu = (struct EraseMenu*)menuItem->data;

    struct MenuItem* result = inputCursorMenu(&eraseMenu->cursor, buttonsDown, 136);

    if (result == eraseMenu->cursor.parentMenu && eraseMenu->cursor.cursorLocation == EraseMenuItemYes)
    {
        if (eraseSaveData() == -1)
        {
            return menuItem;
        }
        else
        {
            return result;
        }
    }

    if (result)
    {
        return result;
    }
    else
    {
        return menuItem;
    }
}