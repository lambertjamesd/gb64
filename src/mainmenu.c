#include "mainmenu.h"
#include "../render.h"
#include "gameboy.h"
#include "../tex/guiitems.h"
#include "debug_out.h"

#define C_BUTTON_BITMAP_COUNT   1

static Bitmap cButtonDown[C_BUTTON_BITMAP_COUNT] = {
    {16, 32, 16, 0, GUIItems, 16, 0},
};

static Bitmap cButtonUp[C_BUTTON_BITMAP_COUNT] = {
    {16, 32, 0, 0, GUIItems, 16, 0},
};

static Gfx      cButtonDL[NUM_DL(C_BUTTON_BITMAP_COUNT)];

Sprite cButtonSprite = {
    0, 0,
    32, 32,
    1.0, 1.0,
    0, 0,
    SP_TRANSPARENT | SP_CUTOUT,
    0x1234,
    255, 255, 255, 255,
    0, 4, (int*)GUIItemsPalette,
    0, 0,
    C_BUTTON_BITMAP_COUNT, NUM_DL(C_BUTTON_BITMAP_COUNT),
    32, 32,
    G_IM_FMT_CI,
    G_IM_SIZ_8b,
    cButtonDown,
    cButtonDL,
    NULL,
};

void saveStateRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    int buttonAlpha = 0;

    struct SaveState* saveState = (struct SaveState*)menuItem->data;

    if (saveState->isLoading)
    {
        ++saveState->showLoadTimer;

        if (saveState->showLoadTimer == LOAD_TIMER_FRAMES)
        {
            loadGameboyState(&gGameboy);
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
        cButtonSprite.bitmap = cButtonUp;
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
        cButtonSprite.bitmap = cButtonDown;
    }

    cButtonSprite.width = 16;
    cButtonSprite.height = 16;

    Gfx *gxp, *dl;
    gxp = glistp;
    cButtonSprite.rsp_dl_next = cButtonSprite.rsp_dl;
    cButtonSprite.alpha = buttonAlpha;
    spClearAttribute(&cButtonSprite, SP_HIDDEN);
    spMove(&cButtonSprite, 8, 12);
    dl = spDraw(&cButtonSprite);
    gSPDisplayList(gxp++, dl);
    glistp = gxp;
}

struct MenuItem* saveStateHandleInput(struct MenuItem* menuItem, int buttonsDown, int buttonsState)
{
    struct SaveState* saveState = (struct SaveState*)menuItem->data;

    if (buttonsDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.save))
    {
        saveGameboyState(&gGameboy);
        saveState->showSaveTimer = SAVE_TIMER_FRAMES;
    }

    if (buttonsDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.load))
    {
        saveState->isLoading = 1;
        if (saveState->showLoadTimer < LOAD_TIMER_START_FRAMES)
        {
            saveState->showLoadTimer = LOAD_TIMER_START_FRAMES;
        }
    }

    return menuItem;
}

struct MenuItem* saveStateHandleUp(struct MenuItem* menuItem, int buttonsUp, int buttonsState)
{
    struct SaveState* saveState = (struct SaveState*)menuItem->data;

    if (buttonsUp & U_CBUTTONS)
    {
        saveState->isLoading = 0;
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

    mainMenu->menuItems[MainMenuItemSaveState].handleButtonUp = saveStateHandleUp;

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