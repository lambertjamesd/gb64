
#ifndef _MAIN_MENU_H
#define _MAIN_MENU_H

#include "menu.h"
#include "bool.h"

#define SAVE_TIMER_FRAMES       96
#define SAVE_TIMER_FADE_TIME    64

#define LOAD_TIMER_START_FRAMES 30
#define LOAD_TIMER_FRAMES       96

#define LEFT_PANEL_WIDTH    80

enum MainMenuItems {
    MainMenuItemSaveState,
    MainMenuItemsCount,
};

struct SaveState {
    u16 showSaveTimer;
    u8 isLoading;
    u8 showLoadTimer;
};

struct MainMenu {
    struct SaveState saveState;
    struct MenuState menu;
    struct MenuItem menuItems[MainMenuItemsCount];
    u16 leftPanelPosition;
    u16 leftPanelTarget;
};

extern struct MainMenu gMainMenu;

void initMainMenu(struct MainMenu* mainMenu);
void updateMainMenu(struct MainMenu* mainMenu, OSContPad* pad);
void renderMainMenu(struct MainMenu* mainMenu);

bool isMainMenuOpen(struct MainMenu* mainMenu);

#endif