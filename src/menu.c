
#include "menu.h"
#include "../memory.h"
#include "debug_out.h"
#include "../render.h"
#include "../tex/textures.h"
#include "gameboy.h"
#include "sprite.h"
#include "spritefont.h"

#define BUTTON_ICON_COUNT   10
#define BUTTON_ICON_DL_LENGTH   60

struct SpriteTile gButtonIconTile[] = {
    {0, 0, 16, 16},
    {16, 0, 16, 16},
    {0, 16, 16, 16},
    {16, 16, 16, 16},
    
    {0, 0, 16, 16},
    {16, 0, 16, 16},
    
    // blank
    {16, 16, 16, 16},
    {16, 16, 16, 16},
    
    {16, 0, 16, 16},
    {0, 0, 16, 16},
    {0, 16, 16, 16},
    {16, 16, 16, 16},
    
    {0, 16, 16, 16},
    {0, 16, 16, 16},
    {16, 0, 16, 16},
    {0, 0, 16, 16},
};

char gButtonIconLayer[] = {
    SPRITE_CBUTTONS_LAYER,
    SPRITE_CBUTTONS_LAYER,
    SPRITE_CBUTTONS_LAYER,
    SPRITE_CBUTTONS_LAYER,
    
    SPRITE_TRIGGERS_LAYER,
    SPRITE_TRIGGERS_LAYER,
    
    // blank
    SPRITE_TRIGGERS_LAYER,
    SPRITE_TRIGGERS_LAYER,
    
    SPRITE_DPAD_LAYER,
    SPRITE_DPAD_LAYER,
    SPRITE_DPAD_LAYER,
    SPRITE_DPAD_LAYER,
    
    SPRITE_FACE_LAYER,
    SPRITE_TRIGGERS_LAYER,
    SPRITE_FACE_LAYER,
    SPRITE_FACE_LAYER,
};

struct SpriteTile gGUIItemTiles[] = {
    {0, 0, 8, 8},
    {8, 0, 8, 8},
    {0, 8, 8, 8},
    {8, 8, 8, 8},
    {16, 0, 8, 8},
    {24, 8, 8, 8},
    {24, 0, 8, 8},
    {16, 8, 8, 8},
    {0, 16, 8, 8},
    {8, 16, 8, 8},
    {24, 24, 8, 8},
};

static Gfx      gButtonIconsDL[NUM_DL(1) * BUTTON_ICON_DL_LENGTH];
static Bitmap   gButtonIcons[BUTTON_ICON_COUNT];

void initMenuState(struct MenuState* menu, struct MenuItem* items, int itemCount)
{
    zeroMemory(menu, sizeof(struct MenuState));
    menu->currentMenuItem = items;
    menu->allItems = items;
    menu->menuItemCount = itemCount;
    menu->holdTimer = ~0;

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
            buttons |= INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.up);
        if (pad->stick_y < -0x40)
            buttons |= INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.down);
        if (pad->stick_x > 0x40)
            buttons |= INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.right);
        if (pad->stick_x < -0x40)
            buttons |= INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.left);
            

        int buttonDown = (menu->lastButtons ^ buttons) & buttons;

        u32 timeMS = (u32)(OS_CYCLES_TO_USEC(osGetTime()) / 1024);

        if (buttonDown || (menu->holdTimer <= timeMS && buttons))
        {
            if (!buttonDown) {
                buttonDown = buttons;
                menu->holdTimer = timeMS + REPEAT_TIME_DELAY;
            } else {
                menu->holdTimer = timeMS + INITIAL_TIME_DELAY;
            }

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
                if ((buttonDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.up)) && menu->currentMenuItem->toUp)
                {
                    menuStateSetActive(menu, menu->currentMenuItem->toUp);
                }
                else if ((buttonDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.down)) && menu->currentMenuItem->toDown)
                {
                    menuStateSetActive(menu, menu->currentMenuItem->toDown);
                }
                else if ((buttonDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.right)) && menu->currentMenuItem->toRight)
                {
                    menuStateSetActive(menu, menu->currentMenuItem->toRight);
                }
                else if ((buttonDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.left)) && menu->currentMenuItem->toLeft)
                {
                    menuStateSetActive(menu, menu->currentMenuItem->toLeft);
                }
            }
        }

        int buttonUp = (menu->lastButtons ^ buttons) & ~buttons;

        if (buttonUp && menu->currentMenuItem->handleButtonUp)
        {
            menu->holdTimer = ~0;
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

///////////////////////////////////

void defaultRenderCursorMenuItem(struct CursorMenuItem* menuItem, int x, int y, int selected)
{
    if (selected)
    {
        spriteDrawTile(
            SPRITE_BORDER_LAYER,
            x, y,
            16, 16,
            gGUIItemTiles[GUIItemIconRight]
        );
    }

    spriteSetColor(gGBFont.spriteLayer, 255, 255, 255, 255);
    renderText(&gGBFont, menuItem->label, x + 24, y, 1);
}

struct MenuItem* defaultInputCursorMenuItem(struct CursorMenuItem* menuItem, int buttonDown)
{
    if (buttonDown & (INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.a) | INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.start)))
    {
        return menuItem->toMenu;
    }

    return NULL;
}

void renderCursorMenu(struct CursorMenu* menu, int x, int y, int height)
{
    int maxY = y + height;
    int menuItemIndex = menu->scrollOffset;

    if (menuItemIndex)
    {
        spriteDrawTile(
            SPRITE_BORDER_LAYER,
            x, y - 16,
            16, 16,
            gGUIItemTiles[GUIItemIconUp]
        );
    }

    while (menuItemIndex < menu->menuItemCount && y < maxY)
    {
        struct CursorMenuItem* menuItem = &menu->menuItems[menuItemIndex];

        if (y + menuItem->height >= maxY)
        {
            break;
        }

        if (menuItem->render)
        {
            menuItem->render(menuItem, x, y, menuItemIndex == menu->cursorLocation);
        }
        else
        {
            defaultRenderCursorMenuItem(menuItem, x, y, menuItemIndex == menu->cursorLocation);
        }

        y += menuItem->height;
        ++menuItemIndex;
    }

    if (menuItemIndex < menu->menuItemCount)
    {
        spriteDrawTile(
            SPRITE_BORDER_LAYER,
            x, y,
            16, 16,
            gGUIItemTiles[GUIItemIconDown]
        );
    }
}

struct MenuItem* inputCursorMenu(struct CursorMenu* menu, int buttons, int height)
{
    struct MenuItem* nextItem;
    struct CursorMenuItem* currentItem = &menu->menuItems[menu->cursorLocation];
    
    if (currentItem->input)
    {
        nextItem = currentItem->input(currentItem, buttons);
    }
    else
    {
        nextItem = defaultInputCursorMenuItem(currentItem, buttons);
    }
    
    if (nextItem)
    {
        return nextItem;
    }

    if (buttons & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.up))
    {
        if (menu->cursorLocation > 0)
        {
            if (menu->scrollOffset == menu->cursorLocation)
            {
                --menu->scrollOffset;
            }

            --menu->cursorLocation;
        }
    }

    if (buttons & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.down))
    {
        if (menu->cursorLocation < menu->menuItemCount - 1)
        {
            int lastItemIndex = menu->scrollOffset;
            int y = 0;
            
            while (lastItemIndex < menu->menuItemCount)
            {
                struct CursorMenuItem* cursorMenuItem = &menu->menuItems[lastItemIndex];

                y += cursorMenuItem->height;

                if (y >= height)
                {
                    break;
                }

                ++lastItemIndex;
            }

            if (menu->cursorLocation + 1 >= lastItemIndex)
            {
                ++menu->scrollOffset;
            }

            ++menu->cursorLocation;
        }
    }

    if (buttons & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.b))
    {
        return menu->parentMenu;
    }

    return NULL;
}

void initCursorMenu(struct CursorMenu* menu, struct CursorMenuItem* menuItems, u16 menuItemCount)
{
    zeroMemory(menu, sizeof(struct CursorMenu));
    menu->menuItems = menuItems;
    menu->menuItemCount = menuItemCount;
}

void initCursorMenuItem(struct CursorMenuItem* item, struct MenuItem* toMenu, char* label, u16 height)
{
    zeroMemory(item, sizeof(struct CursorMenuItem));
    item->toMenu = toMenu;
    item->label = label;
    item->height = height;
}

void renderMenuBorder()
{
    spriteDrawTile(SPRITE_BORDER_LAYER, 0, 80, 192, 16, gGUIItemTiles[GUIItemIconHorz]);
    spriteDrawTile(SPRITE_BORDER_LAYER, 192, 80, 16, 16, gGUIItemTiles[GUIItemIconTopRight]);
    spriteDrawTile(SPRITE_BORDER_LAYER, 192, 96, 16, 288, gGUIItemTiles[GUIItemIconVert]);
    spriteDrawTile(SPRITE_BORDER_LAYER, 192, 384, 16, 16, gGUIItemTiles[GUIItemIconBottomRight]);
    spriteDrawTile(SPRITE_BORDER_LAYER, 0, 384, 192, 16, gGUIItemTiles[GUIItemIconHorz]);
    spriteDrawTile(SPRITE_BORDER_LAYER, 0, 96, 192, 288, gGUIItemTiles[GUIItemIconBlack]);
}

///////////////////////////////////

void initSelectCursorMenuItem(struct SelectCursorMenuItem* item, int id, int value, int minValue, int maxValue, char** labels)
{
    item->changeCallback = NULL;
    item->id = id;
    item->value = value;
    item->minValue = minValue;
    item->maxValue = maxValue;
    item->labels = labels;
}

void renderSelectCursorMenuItem(struct CursorMenuItem* menuItem, int x, int y, int selected)
{
    struct SelectCursorMenuItem* select = (struct SelectCursorMenuItem*)menuItem->data;

    spriteSetColor(gGBFont.spriteLayer, 255, 255, 255, 255);
    renderText(&gGBFont, menuItem->label, x, y, 1);
    
    if (selected)
    {
        spriteDrawTile(SPRITE_BORDER_LAYER, x, y + 24, 16, 16, gGUIItemTiles[GUIItemIconLeft]);
        spriteDrawTile(SPRITE_BORDER_LAYER, x + 120, y + 24, 16, 16, gGUIItemTiles[GUIItemIconRight]);
    }

    int currentValue = (select->value - select->minValue) % (select->maxValue - select->minValue);
    
    if (select->labels)
    {
        renderText(&gGBFont, select->labels[currentValue], x + 24, y + 24, 1);
    }
    else
    {
        char tmpString[16];
        sprintf(tmpString, "%d", select->value);
        renderText(&gGBFont, tmpString, x + 24, y + 24, 1);
    }
}

struct MenuItem* inputSelectCursorMenuItem(struct CursorMenuItem* menuItem, int buttonDown)
{
    struct SelectCursorMenuItem* select = (struct SelectCursorMenuItem*)menuItem->data;

    if (buttonDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.right))
    {
        ++select->value;
        
        if (select->value >= select->maxValue)
        {
            select->value = select->minValue;
        }

        if (select->changeCallback)
        {
            select->changeCallback(menuItem, select->id, select->value);
        }
    }

    if (buttonDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.left))
    {
        --select->value;
        
        if (select->value < select->minValue)
        {
            select->value = select->maxValue - 1;
        }

        if (select->changeCallback)
        {
            select->changeCallback(menuItem, select->id, select->value);
        }
    }

    return NULL;
}