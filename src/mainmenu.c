#include "mainmenu.h"
#include "../render.h"
#include "gameboy.h"
#include "../tex/cbuttons.h"
#include "../tex/dpad.h"
#include "../tex/facebuttons.h"
#include "../tex/guiitems.h"
#include "../tex/triggers.h"
#include "debug_out.h"

#define C_BUTTON_BITMAP_COUNT   1

static Bitmap cButtonRight[C_BUTTON_BITMAP_COUNT] = {
    {16, 32, 0, 0, tex_cbuttons, 16, 0},
};

static Bitmap cButtonLeft[C_BUTTON_BITMAP_COUNT] = {
    {16, 32, 16, 0, tex_cbuttons, 16, 0},
};

static Bitmap cButtonDown[C_BUTTON_BITMAP_COUNT] = {
    {16, 32, 0, 16, tex_cbuttons, 16, 0},
};

static Bitmap cButtonUp[C_BUTTON_BITMAP_COUNT] = {
    {16, 32, 16, 16, tex_cbuttons, 16, 0},
};

static Gfx      cButtonDL[NUM_DL(C_BUTTON_BITMAP_COUNT)];

unsigned short gGUIPallete[] = {
	0x0001,
	0xEF2B,
	0xEE87,
	0xBC83,
	0x1,
	0xC631,
	0x6B5D,
	0x2109,
	0x1,
	0xBE3F,
	0xCFB1,
	0x1935,
	0x1E05,
	0x2055,
	0x2C5,
	0xF5EF,
	0xC885,
	0x5045,
	0x1,
	0xEF2B,
	0x1,
	0xC631,
	0x6B5D,
	0x2109,
};

Sprite cButtonSprite = {
    0, 0,
    32, 32,
    1.0, 1.0,
    0, 0,
    SP_TRANSPARENT | SP_CUTOUT,
    0x1234,
    255, 255, 255, 255,
    0, 4, (int*)gGUIPallete,
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

    if (!gGameboy.memory.misc.biosLoaded)
    {
        if (buttonsDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.save))
        {
            saveGameboyState(&gGameboy);
            saveState->showSaveTimer = SAVE_TIMER_FRAMES;
            
            if (!saveState->isLoading)
            {
                saveState->showLoadTimer = 0;
            }
        }

        if (buttonsDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.load))
        {
            saveState->isLoading = 1;
            if (saveState->showLoadTimer < LOAD_TIMER_START_FRAMES)
            {
                saveState->showLoadTimer = LOAD_TIMER_START_FRAMES;
            }
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