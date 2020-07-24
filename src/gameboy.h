
#ifndef _GAMEBOY_H
#define _GAMEBOY_H

#include <ultra64.h>
#include "rom.h"
#include "cpu.h"
#include "memory_map.h"
#include "audio.h"

#define GB_BUTTON_RIGHT     0x01
#define GB_BUTTON_LEFT      0x02
#define GB_BUTTON_UP        0x04
#define GB_BUTTON_DOWN      0x08

#define GB_BUTTON_A         0x10
#define GB_BUTTON_B         0x20
#define GB_BUTTON_SELECT    0x40
#define GB_BUTTON_START     0x80

#define CPU_TICKS_PER_FRAME     781250
#define CPU_TICKS_PER_SECOND    0x100000
#define MAX_FRAME_SKIP      8

enum InputButtonSetting
{
    InputButtonSetting_RC,
    InputButtonSetting_LC,
    InputButtonSetting_DC,
    InputButtonSetting_UC,
    
    InputButtonSetting_R,
    InputButtonSetting_L,

    InputButtonSetting_RD = 8,
    InputButtonSetting_LD,
    InputButtonSetting_DD,
    InputButtonSetting_UD,
    
    InputButtonSetting_START,
    InputButtonSetting_Z,
    InputButtonSetting_B,
    InputButtonSetting_A,
};

#define INPUT_BUTTON_TO_MASK(inputButton) (1 << (inputButton))

enum InputButtonIndex
{
    InputButtonIndexRight,
    InputButtonIndexLeft,
    InputButtonIndexUp,
    InputButtonIndexDown,
    InputButtonIndexA,
    InputButtonIndexB,
    InputButtonIndexSelect,
    InputButtonIndexStart,
    InputButtonIndexSave,
    InputButtonIndexLoad,
    InputButtonIndexOpenMenu,
    InputButtonIndexCount,
};

struct InputMapping
{
    u8 right;
    u8 left;
    u8 up;
    u8 down;
    u8 a;
    u8 b;
    u8 select;
    u8 start;

    u8 save;
    u8 load;
    u8 openMenu;
    u8 fastForward;

    u32 reserved2;
};

enum ScreenScaleSetting
{
    ScreenScale1,
    ScreenScale1_25,
    ScreenScale1_5,
    ScreenScaleSettingCount,    
};

struct GameboyGraphicsSettings
{
    u32 unused:27;
    u32 smooth:1;
    u32 scaleSetting:4;
};

#define GB_SETTINGS_FLAGS_DISABLE_GBC   0x1

#define GB_SETTINGS_HEADER              0x47423634
#define GB_SETTINGS_CURRENT_VERSION     0

struct GameboySettings
{
    // Always has the value 0x47423634 (GB64 as an ascii string)
    u32 header;
    // Used to check save file compatibility 
    u32 version;
    u16 flags;
    // color palette to use for non color games
    u16 bgpIndex;
    u16 obp0Index;
    u16 obp1Index;
    struct InputMapping inputMapping;
    struct GameboyGraphicsSettings graphics;
};

struct GameBoy
{
    struct GameboySettings settings;
    struct Memory memory;
    struct CPUState cpu;
};

extern struct GameBoy gGameboy;
extern u16 gBGPColors;
extern u16 gOBP0Colors;
extern u16 gOBP1Colors;
extern u32 gPalleteDirty;

void initGameboy(struct GameBoy* gameboy, struct ROMLayout* rom);

void requestInterrupt(struct GameBoy* gameboy, int interrupt);

/**
 * if targetMemory is null then the drawing routines are skipped
 */
void emulateFrame(struct GameBoy* gameboy, bool targetMemory);

void handleGameboyInput(struct GameBoy* gameboy, OSContPad* pad);
void unloadBIOS(struct Memory* memory);
int loadGameboyState(struct GameBoy* gameboy);
int saveGameboyState(struct GameBoy* gameboy);

enum InputButtonSetting getButtonMapping(struct InputMapping* inputMapping, enum InputButtonIndex buttonIndex);
void setButtonMapping(struct InputMapping* inputMapping, enum InputButtonIndex buttonIndex, enum InputButtonSetting setting);

int getPaletteCount();
u16* getPalette(int index);
void updatePaletteInfo(struct GameBoy* gameboy);

#endif