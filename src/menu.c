
#include "menu.h"
#include "../memory.h"
#include "debug_out.h"

void initMenuState(struct MenuState* menu, struct MenuItem* items, int itemCount)
{
    zeroMemory(menu, sizeof(struct MenuState));
    menu->currentMenuItem = items;
    menu->allItems = items;
    menu->menuItemCount = itemCount;

    if (menu->currentMenuItem->setActive)
    {
        menu->currentMenuItem->setActive(menu->currentMenuItem, 1);
    }
}

void menuStateSetActive(struct MenuState* menu, struct MenuItem* item)
{
    if (item && item != menu->currentMenuItem)
    {
        if (menu->currentMenuItem->setActive)
        {
            menu->currentMenuItem->setActive(menu->currentMenuItem, 0);
        }
        menu->currentMenuItem = item;
        if (menu->currentMenuItem->setActive)
        {
            menu->currentMenuItem->setActive(menu->currentMenuItem, 1);
        }
    }
}

void menuStateHandleInput(struct MenuState* menu, OSContPad* pad)
{
    if (menu->currentMenuItem)
    {
        int buttons = pad->button;

        if (pad->stick_y > 0x40)
            buttons |= U_JPAD;
        if (pad->stick_y < -0x40)
            buttons |= D_JPAD;
        if (pad->stick_x > 0x40)
            buttons |= R_JPAD;
        if (pad->stick_x < -0x40)
            buttons |= L_JPAD;
            

        int buttonDown = (menu->lastButtons ^ buttons) & buttons;

        if (buttonDown)
        {
            menu->holdTimer = INITIAL_TIME_DELAY;

            struct MenuItem* nextItem;
            
            if (menu->currentMenuItem->handleButtonDown)
            {
                nextItem = menu->currentMenuItem->handleButtonDown(menu->currentMenuItem, buttonDown, buttons);
            }

            if (nextItem)
            {
                menuStateSetActive(menu, nextItem);
                menu->currentMenuItem = nextItem;
            }
            else
            {
                if ((buttonDown & U_JPAD) && menu->currentMenuItem->toUp)
                {
                    menuStateSetActive(menu, menu->currentMenuItem->toUp);
                }
                else if ((buttonDown & D_JPAD) && menu->currentMenuItem->toDown)
                {
                    menuStateSetActive(menu, menu->currentMenuItem->toDown);
                }
                else if ((buttonDown & R_JPAD) && menu->currentMenuItem->toRight)
                {
                    menuStateSetActive(menu, menu->currentMenuItem->toRight);
                }
                else if ((buttonDown & L_JPAD) && menu->currentMenuItem->toLeft)
                {
                    menuStateSetActive(menu, menu->currentMenuItem->toLeft);
                }
            }
        }
        else if (menu->lastButtons)
        {
            if (--menu->holdTimer == 0)
            {
                menu->holdTimer = REPEAT_TIME_DELAY;
                buttons = 0;
            }
        }

        int buttonUp = (menu->lastButtons ^ buttons) & ~buttons;

        if (buttonUp && menu->currentMenuItem->handleButtonUp)
        {
            menu->currentMenuItem->handleButtonUp(menu->currentMenuItem, buttonUp, buttons);
        }

        menu->lastButtons = buttons;
    }
}

void menuStateRender(struct MenuState* menu)
{
    int index;

    for (index = 0; index < menu->menuItemCount; ++index)
    {
        if (!(menu->allItems[index].flags & MENU_ITEM_FLAGS_HIDDEN))
        {
            menu->allItems[index].renderCallback(&menu->allItems[index], menu->currentMenuItem);
        }
    }
}

void menuItemInit(struct MenuItem* menuItem, void* data, MenuItemRender renderCallback, MenuItemHandleInput handleButtonDown, MenuItemSetActive setActive)
{
    menuItem->data = data;
    menuItem->renderCallback = renderCallback;
    menuItem->handleButtonDown = handleButtonDown;
    menuItem->handleButtonUp = NULL;
    menuItem->setActive = setActive;
}

void menuItemConnectSideToSide(struct MenuItem* left, struct MenuItem* right)
{
    left->toRight = right;
    right->toLeft = left;
}

void menuItemConnectUpAndDown(struct MenuItem* up, struct MenuItem* down)
{
    up->toDown = down;
    down->toUp = up;
}