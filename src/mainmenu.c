#include "mainmenu.h"
#include "../render.h"
#include "gameboy.h"

void saveStateRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    struct SaveState* saveState = (struct SaveState*)menuItem->data;

    if (saveState->showSaveTimer)
    {
        if (saveState->showSaveTimer > SAVE_TIMER_FADE_TIME)
        {
            FONTCOL(40, 255, 0, 255);
        }
        else
        {
            FONTCOL(
                40,
                255,
                0,
                255 * ((saveState->showSaveTimer + 0x1F) & 0x60)  / SAVE_TIMER_FADE_TIME
            );
        }

        SHOWFONT(&glistp, "SAVED", 32, 16);
    }
}

struct MenuItem* saveStateHandleInput(struct MenuItem* menuItem, int buttonsDown, int buttonsState)
{
    struct SaveState* saveState = (struct SaveState*)menuItem->data;

    if (buttonsDown & D_CBUTTONS)
    {
        saveRAM(&gGameboy.memory);
        saveState->showSaveTimer = SAVE_TIMER_FRAMES;
    }

    return menuItem;
}

void initMainMenu(struct MainMenu* mainMenu)
{
    menuItemInit(
        &mainMenu->menuItems[MainMenuItemSaveState],
        &mainMenu->saveState,
        saveStateRender,
        saveStateHandleInput,
        NULL
    );

    initMenuState(&mainMenu->menu, mainMenu->menuItems, MainMenuItemsCount);
}

void updateMainMenu(struct MainMenu* mainMenu, OSContPad* pad)
{
    menuStateHandleInput(&mainMenu->menu, pad);
    
    if (mainMenu->saveState.showSaveTimer)
    {
        --mainMenu->saveState.showSaveTimer;
    }
}

void renderMainMenu(struct MainMenu* mainMenu)
{
    menuStateRender(&mainMenu->menu);
}


bool isMainMenuOpen(struct MainMenu* mainMenu)
{
    return mainMenu->leftPanelPosition != 0;
}

struct MainMenu gMainMenu;