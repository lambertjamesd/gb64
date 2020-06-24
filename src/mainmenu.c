#include "mainmenu.h"
#include "../render.h"
#include "gameboy.h"
#include "../tex/guiitems.h"

#define C_BUTTON_BITMAP_COUNT   1

static Bitmap cButtonBM[C_BUTTON_BITMAP_COUNT] = {
    {16, 16, 0, 0, GUIItems, 16, 0},
};

static Gfx      cButtonDL[NUM_DL(C_BUTTON_BITMAP_COUNT)];

Sprite cButtonSprite = {
    0, 0,
    0, 0,
    1.0, 1.0,
    0, 0,
    SP_TRANSPARENT,
    0x1234,
    255, 255, 255, 255,
    0, 4, (int*)GUIItemsPalette,
    0, 1,
    C_BUTTON_BITMAP_COUNT, NUM_DL(C_BUTTON_BITMAP_COUNT),
    16, 32,
    G_IM_FMT_CI,
    G_IM_SIZ_8b,
    cButtonBM,
    cButtonDL,
    NULL,
};

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

    cButtonSprite.width = 16;
    cButtonSprite.height = 16;

    // Gfx *gxp, *dl;
    // gxp = glistp;
    // dl = spDraw( &cButtonSprite );
    // gSPDisplayList( gxp++, dl );
    // glistp = gxp;
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
    cButtonSprite.rsp_dl_next = cButtonSprite.rsp_dl;

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