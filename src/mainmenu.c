#include "mainmenu.h"
#include "../render.h"
#include "gameboy.h"
#include "debug_out.h"
#include "save.h"
#include "version.h"
#include "spritefont.h"
#include "sprite.h"

///////////////////////////////////

void saveStateRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    int buttonAlpha = 0;

    struct SaveState* saveState = (struct SaveState*)menuItem->data;

    if (saveState->isLoading)
    {
        if (saveState->showLoadTimer < LOAD_TIMER_FRAMES)
        {
            ++saveState->showLoadTimer;
        }

        enum StoredInfoType saveType = getStoredInfoType(&gGameboy);

        if (saveState->showLoadTimer == LOAD_TIMER_FRAMES && saveType == StoredInfoTypeAll)
        {
            if (gGameboy.memory.misc.biosLoaded)
            {
                unloadBIOS(&gGameboy.memory);
            }

            if (loadGameboyState(&gGameboy, saveType))
            {
                saveState->loadMessage = "Failed to load save state";
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
        spriteSetColor(gGBFont.spriteLayer, 255, 255, 255, buttonAlpha);
        renderText(&gGBFont, saveState->loadMessage, 32, 16, 0);
        char layer = gButtonIconLayer[gGameboy.settings.inputMapping.load];
        spriteSetColor(layer, 255, 255, 255, buttonAlpha);
        spriteDrawTile(layer, 8, 12, 16, 16, gButtonIconTile[gGameboy.settings.inputMapping.load]);
    }
    else if (saveState->showSaveTimer)
    {
        if (saveState->showSaveTimer > SAVE_TIMER_FADE_TIME)
        {
            buttonAlpha = 255;
            spriteSetColor(gGBFont.spriteLayer, 40, 255, 0, 255);
        }
        else
        {
            buttonAlpha = 255 * saveState->showSaveTimer / SAVE_TIMER_FADE_TIME;
            spriteSetColor(gGBFont.spriteLayer, 
                40,
                255,
                0,
                buttonAlpha
            );
        }

        renderText(&gGBFont, saveState->saveMessage, 32, 16, 0);

        char layer = gButtonIconLayer[gGameboy.settings.inputMapping.save];
        spriteSetColor(layer, 255, 255, 255, buttonAlpha);
        spriteDrawTile(layer, 8, 12, 16, 16, gButtonIconTile[gGameboy.settings.inputMapping.save]);
    }
    else if (saveState->showFastTimer > 0)
    {
        if (saveState->showFastTimer > SAVE_TIMER_FADE_TIME)
        {
            buttonAlpha = 255;
            spriteSetColor(gGBFont.spriteLayer, 40, 255, 0, 255);
        }
        else
        {
            buttonAlpha = 255 * saveState->showFastTimer / SAVE_TIMER_FADE_TIME;
            spriteSetColor(gGBFont.spriteLayer, 
                40,
                255,
                0,
                buttonAlpha
            );
        }

        renderText(&gGBFont, "FAST FORWARD", 32, 16, 0);
        char layer = gButtonIconLayer[gGameboy.settings.inputMapping.fastForward];
        spriteSetColor(layer, 255, 255, 255, buttonAlpha);
        spriteDrawTile(layer, 8, 12, 16, 16, gButtonIconTile[gGameboy.settings.inputMapping.fastForward]);
    }
}

struct MenuItem* saveStateHandleInput(struct MenuItem* menuItem, int buttonsDown, int buttonsState)
{
    struct SaveState* saveState = (struct SaveState*)menuItem->data;

    if (buttonsDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.load))
    {
        enum StoredInfoType saveType = getStoredInfoType(&gGameboy);

        if (saveType == StoredInfoTypeAll)
        {
            saveState->loadMessage = "HOLD TO LOAD";
            saveState->isLoading = 1;
            if (saveState->showLoadTimer < LOAD_TIMER_START_FRAMES)
            {
                saveState->showLoadTimer = LOAD_TIMER_START_FRAMES;
            }
        }
        else
        {
            saveState->loadMessage = "Insufficient memory";
            saveState->isLoading = 1;
            saveState->showLoadTimer = LOAD_TIMER_FRAMES;
        }
    }

    if (!gGameboy.memory.misc.biosLoaded && (buttonsDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.save)))
    {
        enum StoredInfoType saveType = saveGameboyState(&gGameboy);

        switch (saveType)
        {
        case StoredInfoTypeAll:
            saveState->saveMessage = "Saved state";
            break;
        case StoredInfoTypeSettingsRAM:
            saveState->saveMessage = "Saved cart w settings";
            break;
        case StoredInfoTypeRAM:
            saveState->saveMessage = "Saved cart";
            break;
        case StoredInfoTypeSettings:
            saveState->saveMessage = "Saved settings";
            break;
        case StoredInfoTypeNone:
            saveState->saveMessage = "Error saving";
            break;
        }
        
        saveState->showSaveTimer = SAVE_TIMER_FRAMES;
        
        if (!saveState->isLoading)
        {
            saveState->showLoadTimer = 0;
        }
    }

    if (buttonsDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.fastForward))
    {
        saveState->isFast = 1;
        saveState->showFastTimer = SAVE_TIMER_FRAMES;
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

    if (buttonsUp & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.fastForward))
    {
        saveState->isFast = 0;
        saveState->showFastTimer = SAVE_TIMER_FADE_TIME;
    }

    return menuItem;
}

///////////////////////////////////

void mainMenuRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    struct MainMenuState* mainMenuState = (struct MainMenuState*)menuItem->data;
    
    if (menuItem == highlightedItem)
    {
        renderMenuBorder();
        spriteSetColor(gGBFont.spriteLayer, 255, 255, 255, 255);

        renderCursorMenu(&mainMenuState->cursorMenu, 20, 56, 160);

        renderText(&gGBFont, EMU_VERSION, 16, 216, 0);
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
    mainMenu->saveState.saveMessage = "Saved";
    mainMenu->saveState.loadMessage = "Loaded";
    
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

    menuItemInit(
        &mainMenu->menuItems[MainMenuItemMainClock],
        &mainMenu->clockMenu,
        clockMenuRender,
        clockMenuHandleInput,
        setClockMenuActive
    );

    initCursorMenu(
        &mainMenu->mainMenuState.cursorMenu, 
        mainMenu->mainMenuState.items,
        (!gGameboy.memory.mbc || (gGameboy.memory.mbc->flags & MBC_FLAGS_TIMER)) ? MainMenuStateItemsCount : MainMenuStateItemsClock
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

    initCursorMenuItem(
        &mainMenu->mainMenuState.items[MainMenuStateItemsClock],
        &mainMenu->menuItems[MainMenuItemMainClock],
        "CLOCK",
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

    initClockMenu(
        &mainMenu->clockMenu,
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
    
    if (mainMenu->saveState.showFastTimer && !mainMenu->saveState.isFast)
    {
        mainMenu->saveState.showFastTimer -= 4;
    }
}

void renderMainMenu(struct MainMenu* mainMenu)
{
    menuStateRender(&mainMenu->menu);
}

bool isMainMenuOpen(struct MainMenu* mainMenu)
{
    return mainMenu->menu.currentMenuItem != mainMenu->menuItems;
}

struct MainMenu gMainMenu;