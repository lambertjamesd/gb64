#include "mainmenu.h"
#include "../render.h"
#include "gameboy.h"
#include "debug_out.h"

///////////////////////////////////

void saveStateRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    int buttonAlpha = 0;
    gButtonSprite.nbitmaps = 0;

    struct SaveState* saveState = (struct SaveState*)menuItem->data;

    if (saveState->isLoading)
    {
        ++saveState->showLoadTimer;

        if (saveState->showLoadTimer == LOAD_TIMER_FRAMES)
        {
            if (gGameboy.memory.misc.biosLoaded)
            {
                unloadBIOS(&gGameboy.memory);
            }

            if (loadGameboyState(&gGameboy))
            {
                DEBUG_PRINT_F("Failed to load save state\n");
            }
            saveState->showLoadTimer = 0;
            saveState->isLoading = 0;
        }
    }
    else if (saveState->showLoadTimer)
    {
        --saveState->showLoadTimer;
    }

    if (saveState->showLoadTimer)
    {
        buttonAlpha = 255 * saveState->showLoadTimer / LOAD_TIMER_FRAMES;
        FONTCOL(255, 255, 255, buttonAlpha);
        SHOWFONT(&glistp, "HOLD TO LOAD", 32, 16);
        gButtonSprite.bitmap[gButtonSprite.nbitmaps++] = gButtonIconTemplates[gGameboy.settings.inputMapping.load];
    }
    else if (saveState->showSaveTimer)
    {
        if (saveState->showSaveTimer > SAVE_TIMER_FADE_TIME)
        {
            buttonAlpha = 255;
            FONTCOL(40, 255, 0, 255);
        }
        else
        {
            buttonAlpha = 255 * saveState->showSaveTimer / SAVE_TIMER_FADE_TIME;
            FONTCOL(
                40,
                255,
                0,
                buttonAlpha
            );
        }

        SHOWFONT(&glistp, "SAVED", 32, 16);
        gButtonSprite.bitmap[gButtonSprite.nbitmaps++] = gButtonIconTemplates[gGameboy.settings.inputMapping.save];
    }

    Gfx *gxp, *dl;
    gxp = glistp;
    gButtonSprite.alpha = buttonAlpha;
    spMove(&gButtonSprite, 8, 12);
    spScale(&gButtonSprite, 1, 1);
    dl = spDraw(&gButtonSprite);
    gSPDisplayList(gxp++, dl);
    glistp = gxp;
}

struct MenuItem* saveStateHandleInput(struct MenuItem* menuItem, int buttonsDown, int buttonsState)
{
    struct SaveState* saveState = (struct SaveState*)menuItem->data;

    if (buttonsDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.load))
    {
        saveState->isLoading = 1;
        if (saveState->showLoadTimer < LOAD_TIMER_START_FRAMES)
        {
            saveState->showLoadTimer = LOAD_TIMER_START_FRAMES;
        }
    }

    if (!gGameboy.memory.misc.biosLoaded && (buttonsDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.save)))
    {
        saveGameboyState(&gGameboy);
        saveState->showSaveTimer = SAVE_TIMER_FRAMES;
        
        if (!saveState->isLoading)
        {
            saveState->showLoadTimer = 0;
        }
    }

    if (buttonsDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.openMenu))
    {
        return saveState->mainMenu;
    }

    return menuItem;
}

struct MenuItem* saveStateHandleUp(struct MenuItem* menuItem, int buttonsUp, int buttonsState)
{
    struct SaveState* saveState = (struct SaveState*)menuItem->data;

    if (buttonsUp & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.load))
    {
        saveState->isLoading = 0;
    }

    return menuItem;
}

///////////////////////////////////

void mainMenuRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    struct MainMenuState* mainMenuState = (struct MainMenuState*)menuItem->data;
    
    if (menuItem == highlightedItem)
    {
        gButtonSprite.alpha = 255;

        renderMenuBorder();
        FONTCOL(255, 255, 255, 255);

        renderCursorMenu(&mainMenuState->cursorMenu, 8, 56, 160);
    }
}

struct MenuItem* mainMenuHandleInput(struct MenuItem* menuItem, int buttonsDown, int buttonsState)
{
    struct MainMenuState* mainMenuState = (struct MainMenuState*)menuItem->data;

    if (buttonsDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.openMenu))
    {
        return mainMenuState->cursorMenu.parentMenu;
    }

    struct MenuItem* result = inputCursorMenu(&mainMenuState->cursorMenu, buttonsDown, 160);

    if (result)
    {
        return result;
    }
    else
    {
        return menuItem;
    }
}

///////////////////////////////////

void initMainMenu(struct MainMenu* mainMenu)
{
    menuItemInit(
        &mainMenu->menuItems[MainMenuItemSaveState],
        &mainMenu->saveState,
        saveStateRender,
        saveStateHandleInput,
        NULL
    );
    mainMenu->menuItems[MainMenuItemSaveState].handleButtonUp = saveStateHandleUp;
    
    menuItemInit(
        &mainMenu->menuItems[MainMenuItemMainMenu],
        &mainMenu->mainMenuState,
        mainMenuRender,
        mainMenuHandleInput,
        NULL
    );
    
    menuItemInit(
        &mainMenu->menuItems[MainMenuItemMainInput],
        &mainMenu->inputMapping,
        inputMappingRender,
        inputMappingHandleInput,
        NULL
    );
    
    menuItemInit(
        &mainMenu->menuItems[MainMenuItemMainScreen],
        &mainMenu->graphicsMenu,
        graphicsMenuRender,
        graphicsMenuHandleInput,
        NULL
    );

    initCursorMenu(
        &mainMenu->mainMenuState.cursorMenu, 
        mainMenu->mainMenuState.items,
        MainMenuStateItemsCount
    );

    initCursorMenuItem(
        &mainMenu->mainMenuState.items[MainMenuStateItemsInput],
        &mainMenu->menuItems[MainMenuItemMainInput],
        "INPUT",
        16
    );

    initCursorMenuItem(
        &mainMenu->mainMenuState.items[MainMenuStateItemsScreen],
        &mainMenu->menuItems[MainMenuItemMainScreen],
        "SCREEN",
        16
    );

    mainMenu->saveState.mainMenu = &mainMenu->menuItems[MainMenuItemMainMenu];
    mainMenu->mainMenuState.cursorMenu.parentMenu = &mainMenu->menuItems[MainMenuItemSaveState];

    initInputMappingMenu(
        &mainMenu->inputMapping,
        &mainMenu->menuItems[MainMenuItemMainMenu]
    );

    initGraphicsMenu(
        &mainMenu->graphicsMenu,
        &mainMenu->menuItems[MainMenuItemMainMenu]
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
    gButtonSprite.rsp_dl_next = gButtonSprite.rsp_dl;
    gButtonSprite.nbitmaps = 0;
    menuStateRender(&mainMenu->menu);
}

bool isMainMenuOpen(struct MainMenu* mainMenu)
{
    return mainMenu->menu.currentMenuItem != mainMenu->menuItems;
}

struct MainMenu gMainMenu;