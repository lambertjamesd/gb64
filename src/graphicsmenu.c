
#include "graphicsmenu.h"
#include "../render.h"
#include "gameboy.h"
#include "graphics.h"
#include "debug_out.h"
#include "spritefont.h"
#include "sprite.h"

static char* gScaleLabels[ScreenScaleSettingCount] = {
    "1",
    "1.25",
    "1.5",
};

static char* gPixelLabels[ScreenScaleSettingCount] = {
    "Sharp",
    "Smooth",
};

void renderPaletteItem(struct CursorMenuItem* menuItem, int x, int y, int selected)
{
    spriteSetColor(gGBFont.spriteLayer, 255, 255, 255, 255);
    renderText(&gGBFont, menuItem->label, x, y, 0);

    u16* paletteIndexPtr = (u16*)menuItem->data;
    u16* palette = getPalette(*paletteIndexPtr);

    if (selected)
    {
        spriteDrawTile(SPRITE_BORDER_LAYER, x, y + 12, 8, 8, gGUIItemTiles[GUIItemIconLeft]);
        spriteDrawTile(SPRITE_BORDER_LAYER, x + 60, y + 12, 8, 8, gGUIItemTiles[GUIItemIconRight]);
    }

    spriteSetColor(SPRITE_BORDER_LAYER, GET_R(palette[0]), GET_G(palette[0]), GET_B(palette[0]), 255);
    spriteDrawTile(SPRITE_BORDER_LAYER, x + 12, y + 12, 8, 8, gGUIItemTiles[GUIItemIconWhite]);
    spriteSetColor(SPRITE_BORDER_LAYER, GET_R(palette[1]), GET_G(palette[1]), GET_B(palette[1]), 255);
    spriteDrawTile(SPRITE_BORDER_LAYER, x + 24, y + 12, 8, 8, gGUIItemTiles[GUIItemIconWhite]);
    spriteSetColor(SPRITE_BORDER_LAYER, GET_R(palette[2]), GET_G(palette[2]), GET_B(palette[2]), 255);
    spriteDrawTile(SPRITE_BORDER_LAYER, x + 36, y + 12, 8, 8, gGUIItemTiles[GUIItemIconWhite]);
    spriteSetColor(SPRITE_BORDER_LAYER, GET_R(palette[3]), GET_G(palette[3]), GET_B(palette[3]), 255);
    spriteDrawTile(SPRITE_BORDER_LAYER, x + 48, y + 12, 8, 8, gGUIItemTiles[GUIItemIconWhite]);

    spriteSetColor(SPRITE_BORDER_LAYER, 255, 255, 255, 255);
}

struct MenuItem* inputPaletteItem(struct CursorMenuItem* menuItem, int buttonDown)
{
    u16* paletteIndexPtr = (u16*)menuItem->data;

    if (buttonDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.right))
    {
        struct GameboySettings prevSettings = gGameboy.settings;
        *paletteIndexPtr = (*paletteIndexPtr + 1) % getPaletteCount();
        updatePaletteInfo(&gGameboy, &prevSettings);
    }

    if (buttonDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.left))
    {
        struct GameboySettings prevSettings = gGameboy.settings;

        if (*paletteIndexPtr == 0)
        {
            *paletteIndexPtr = getPaletteCount() - 1;
        }
        else
        {
            *paletteIndexPtr = *paletteIndexPtr - 1;
        }
        updatePaletteInfo(&gGameboy, &prevSettings);
    }

    return NULL;
}

void graphicsMenuRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    struct GraphicsMenu* graphicsMenu = (struct GraphicsMenu*)menuItem->data;
    
    if (menuItem == highlightedItem)
    {
        renderMenuBorder();
        spriteSetColor(gGBFont.spriteLayer, 255, 255, 255, 255);
        renderText(&gGBFont, "SCREEN", 16, 32, 0);
        renderCursorMenu(&graphicsMenu->cursor, 16, 56, 136);
    }
}

struct MenuItem* graphicsMenuHandleInput(struct MenuItem* menuItem, int buttonsDown, int buttonsState)
{
    struct GraphicsMenu* graphicsMenu = (struct GraphicsMenu*)menuItem->data;
    if (buttonsDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.openMenu))
    {
        return graphicsMenu->cursor.parentMenu;
    }

    struct MenuItem* result = inputCursorMenu(&graphicsMenu->cursor, buttonsDown, 136);

    if (result)
    {
        return result;
    }
    else
    {
        return menuItem;
    }
}

void setScaleSetting(struct CursorMenuItem* item, int id, int value)
{
    gGameboy.settings.graphics.scaleSetting = value;
}

void setPixelSetting(struct CursorMenuItem* item, int id, int value)
{
    gGameboy.settings.graphics.smooth = value;
}

void initGraphicsMenu(struct GraphicsMenu* menu, struct MenuItem* parentMenu)
{
    initCursorMenu(
        &menu->cursor,
        menu->menuItems,
        gGameboy.cpu.gbc ? GraphicsMenuItemGBP : GraphicsMenuItemCount
    );
    menu->cursor.parentMenu = parentMenu;

    initCursorMenuItem(
        &menu->menuItems[GraphicsMenuItemScale],
        NULL,
        "Scale",
        32
    );
    initSelectCursorMenuItem(
        &menu->scale, 
        GraphicsMenuItemScale, 
        gGameboy.settings.graphics.scaleSetting, 
        0, ScreenScaleSettingCount, 
        gScaleLabels
    );
    menu->menuItems[GraphicsMenuItemScale].data = &menu->scale;
    menu->menuItems[GraphicsMenuItemScale].render = renderSelectCursorMenuItem;
    menu->menuItems[GraphicsMenuItemScale].input = inputSelectCursorMenuItem;
    menu->scale.changeCallback = setScaleSetting;
    
    initCursorMenuItem(
        &menu->menuItems[GraphicsMenuItemPixel],
        NULL,
        "Pixel",
        32
    );
    initSelectCursorMenuItem(
        &menu->pixel, 
        GraphicsMenuItemPixel, 
        gGameboy.settings.graphics.smooth, 
        0, 2,
        gPixelLabels
    );
    menu->menuItems[GraphicsMenuItemPixel].data = &menu->pixel;
    menu->menuItems[GraphicsMenuItemPixel].render = renderSelectCursorMenuItem;
    menu->menuItems[GraphicsMenuItemPixel].input = inputSelectCursorMenuItem;
    menu->pixel.changeCallback = setPixelSetting;

    initCursorMenuItem(
        &menu->menuItems[GraphicsMenuItemGBP],
        NULL,
        "BGP",
        32
    );
    menu->menuItems[GraphicsMenuItemGBP].data = &gGameboy.settings.bgpIndex;
    menu->menuItems[GraphicsMenuItemGBP].render = renderPaletteItem;
    menu->menuItems[GraphicsMenuItemGBP].input = inputPaletteItem;
    
    initCursorMenuItem(
        &menu->menuItems[GraphicsMenuItemOBP0],
        NULL,
        "OBP0",
        32
    );
    menu->menuItems[GraphicsMenuItemOBP0].data = &gGameboy.settings.obp0Index;
    menu->menuItems[GraphicsMenuItemOBP0].render = renderPaletteItem;
    menu->menuItems[GraphicsMenuItemOBP0].input = inputPaletteItem;

    initCursorMenuItem(
        &menu->menuItems[GraphicsMenuItemOBP1],
        NULL,
        "OBP1",
        32
    );
    menu->menuItems[GraphicsMenuItemOBP1].data = &gGameboy.settings.obp1Index;
    menu->menuItems[GraphicsMenuItemOBP1].render = renderPaletteItem;
    menu->menuItems[GraphicsMenuItemOBP1].input = inputPaletteItem;
}