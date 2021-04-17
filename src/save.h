
#ifndef _SAVE_H
#define _SAVE_H

#include <ultra64.h>
#include "gameboy.h"

enum SaveType
{
    SaveTypeFlash,
    SaveTypeSRAM,
    SaveTypeSRAM3X,
};

#define SAVE_HEADER_VALUE   0x53564944

struct SaveTypeSetting
{
    u32 header; // always SAVE_HEADER_VALUE to allow rom wrapper to find this setting
    enum SaveType saveType;
};

extern struct SaveTypeSetting gSaveTypeSetting;
typedef int (*SaveReadCallback)(void* target, int sramOffset, int length);
typedef int (*SaveWriteCallback)(void *from, int sramOffset, int length);

enum StoredInfoType loadSettings(struct GameBoy* gameboy);
int loadGameboyState(struct GameBoy* gameboy, enum StoredInfoType storeType);
void loadRAM(struct Memory* memory, enum StoredInfoType storeType, int compressedSize);
enum StoredInfoType saveGameboyState(struct GameBoy* gameboy);
int getSaveStateSize(struct GameBoy* gameboy);
void initSaveCallbacks();
enum StoredInfoType getStoredInfoType(struct GameBoy* gameboy);
enum StoredInfoType getDeprecatedStoredInfoType(struct GameBoy* gameboy);

#endif