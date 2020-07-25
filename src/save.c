
#include "save.h"
#include "gameboy.h"
#include "../memory.h"
#include "debug_out.h"
#include "upgrade.h"

struct SaveTypeSetting gSaveTypeSetting = {
    SAVE_HEADER_VALUE,
    SaveTypeFlash,
};

extern OSMesgQueue     dmaMessageQ;
extern OSMesg          dmaMessageBuf;
extern OSPiHandle	   *handler;
extern OSIoMesg        dmaIOMessageBuf;

#define FLASH_BLOCK_SIZE    0x80

#define ALIGN_FLASH_OFFSET(offset) (((offset) + 0x7F) & ~0x7F)

#define SRAM_ADDR   0x08000000

char gFlashTmpBuffer[FLASH_BLOCK_SIZE];
SaveReadCallback gSaveReadCallback;
SaveWriteCallback gSaveWriteCallback;

int loadFromSRAM(void* target, int sramOffset, int length)
{
	OSIoMesg dmaIoMesgBuf;

    dmaIoMesgBuf.hdr.pri = OS_MESG_PRI_HIGH;
    dmaIoMesgBuf.hdr.retQueue = &dmaMessageQ;
    dmaIoMesgBuf.dramAddr = target;
    dmaIoMesgBuf.devAddr = SRAM_ADDR + sramOffset * FLASH_BLOCK_SIZE;
    dmaIoMesgBuf.size = length;

    osInvalDCache(target, length);
    osEPiStartDma(handler, &dmaIoMesgBuf, OS_READ);
	(void) osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
}

int saveToSRAM(void* target, int sramOffset, int length)
{
	OSIoMesg dmaIoMesgBuf;

    dmaIoMesgBuf.hdr.pri = OS_MESG_PRI_HIGH;
    dmaIoMesgBuf.hdr.retQueue = &dmaMessageQ;
    dmaIoMesgBuf.dramAddr = target;
    dmaIoMesgBuf.devAddr = SRAM_ADDR + sramOffset * FLASH_BLOCK_SIZE;
    dmaIoMesgBuf.size = length;

    osWritebackDCache(target, length);
    osEPiStartDma(handler, &dmaIoMesgBuf, OS_WRITE);
	(void) osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
}

int loadFromFlash(void* target, int sramOffset, int length)
{
    OSIoMesg dmaIoMesgBuf;

    if (length / FLASH_BLOCK_SIZE > 0)
    {
        osInvalDCache(target, (length & ~0x7F));
        if (osFlashReadArray(
                &dmaIoMesgBuf, 
                OS_MESG_PRI_NORMAL, 
                sramOffset / FLASH_BLOCK_SIZE,
                target,
                length / FLASH_BLOCK_SIZE,
                &dmaMessageQ
            ) == -1) 
        {
            return -1;
        }
        (void) osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
    }

    if (length % FLASH_BLOCK_SIZE != 0)
    {
        osInvalDCache(gFlashTmpBuffer, FLASH_BLOCK_SIZE);
        if (osFlashReadArray(
                &dmaIoMesgBuf, 
                OS_MESG_PRI_NORMAL, 
                sramOffset / FLASH_BLOCK_SIZE + length / FLASH_BLOCK_SIZE,
                gFlashTmpBuffer,
                1,
                &dmaMessageQ
            )) 
        {
            return -1;
        }
        (void) osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
        memCopy((char*)target + (length & ~0x7F), gFlashTmpBuffer, length % FLASH_BLOCK_SIZE);
    }

    return 0;
}

int saveToFlash(void *from, int sramOffset, int length)
{
    while (length > 0)
    {
        OSIoMesg dmaIoMesgBuf;
        int pageNumber = sramOffset / FLASH_BLOCK_SIZE;

        if (length < FLASH_BLOCK_SIZE)
        {
            memCopy(gFlashTmpBuffer, from, length);
            zeroMemory(gFlashTmpBuffer + length, FLASH_BLOCK_SIZE - length);
            from = gFlashTmpBuffer;
        }

        osWritebackDCache(from, FLASH_BLOCK_SIZE);
        if (osFlashWriteBuffer(
                &dmaIoMesgBuf, 
                OS_MESG_PRI_NORMAL,
                from,
                &dmaMessageQ
            ) == -1
        ) 
        {
            return -1;
        }
        (void) osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
        if (osFlashWriteArray(pageNumber) == -1)
        {
            return -1;
        }

        from = (char*)from + FLASH_BLOCK_SIZE;
        sramOffset += FLASH_BLOCK_SIZE;
        length -= FLASH_BLOCK_SIZE;
    }

    return 0;
}

void loadRAM(struct Memory* memory, enum StoredInfoType storeType)
{
    if (memory->mbc && memory->mbc->flags | MBC_FLAGS_BATTERY)
    {
        int size = RAM_BANK_SIZE * getRAMBankCount(memory->rom);
        switch (storeType)
        {
            case StoredInfoTypeAll:
            case StoredInfoTypeSettingsRAM:
                gSaveReadCallback(
                    memory->cartRam, 
                    ALIGN_FLASH_OFFSET(sizeof(struct GameboySettings)), 
                    size
                );
                break;
            case StoredInfoTypeRAM:
                gSaveReadCallback(
                    memory->cartRam, 
                    0, 
                    size
                );
                break;
        }
    }
}

void loadSettings(struct GameBoy* gameboy, enum StoredInfoType storeType)
{
    if (storeType == StoredInfoTypeAll || 
        storeType == StoredInfoTypeSettingsRAM ||
        storeType == StoredInfoTypeSettings)
    {
        gSaveReadCallback(&gameboy->settings, 0, sizeof(struct GameboySettings));
    }
    else
    {
        gameboy->settings = gDefaultSettings;
    }
}

struct MiscSaveStateData
{
    struct CPUState cpu;
    char cpuPadding[128 - sizeof(struct CPUState)];
    struct AudioRenderState audioState;
};

int loadGameboyState(struct GameBoy* gameboy, enum StoredInfoType storeType)
{
    if (storeType != StoredInfoTypeAll)
    {
        return -1;
    }

    int offset = 0;
    int sectionSize;
    struct GameboySettings settings;
    sectionSize = sizeof(struct GameboySettings);
    if (gSaveReadCallback(&settings, offset, sectionSize))
    {
        DEBUG_PRINT_F("Could not load header\n");
        return -1;
    }
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

    if (settings.header != GB_SETTINGS_HEADER || settings.version > GB_SETTINGS_CURRENT_VERSION)
    {
        DEBUG_PRINT_F("Invalid header %X %X\n", settings.header, settings.version);
        return -1;
    }

    gameboy->settings = settings;

    sectionSize = RAM_BANK_SIZE * getRAMBankCount(gameboy->memory.rom);
    if (gSaveReadCallback(gameboy->memory.cartRam, offset, sectionSize) == -1)
    {
        return -1;
    }
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

    sectionSize = sizeof(struct GraphicsMemory);
    if (gSaveReadCallback(&gameboy->memory.vram, offset, sectionSize) == -1)
    {
        return -1;
    }
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

    sectionSize = MAX_RAM_SIZE;
    if (gSaveReadCallback(gameboy->memory.internalRam, offset, sectionSize) == -1)
    {
        return -1;
    }
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);
    
    sectionSize = sizeof(struct MiscMemory);
    if (gSaveReadCallback(&gameboy->memory.misc, offset, sectionSize) == -1)
    {
        return -1;
    }
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

    struct MiscSaveStateData miscData;
    sectionSize = sizeof(struct MiscSaveStateData);
    if (gSaveReadCallback(&miscData, offset, sectionSize) == -1)
    {
        return -1;
    }

    gameboy->cpu = miscData.cpu;
    gameboy->memory.audio = miscData.audioState;
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

    gameboy->memory.bankSwitch(&gameboy->memory, -1, 0);

    if (gameboy->cpu.gbc)
    {
        int bank = READ_REGISTER_DIRECT(&gameboy->memory, REG_VBK) & REG_VBK_MASK;
        gameboy->memory.memoryMap[0x8] = gameboy->memory.vramBytes + (bank ? MEMORY_MAP_SEGMENT_SIZE * 2 : 0);
        gameboy->memory.memoryMap[0x9] = gameboy->memory.memoryMap[0x8] + MEMORY_MAP_SEGMENT_SIZE;

        bank = READ_REGISTER_DIRECT(&gameboy->memory, REG_SVBK) & REG_SVBK_MASK;
        gameboy->memory.memoryMap[0xD] = bank ?
            gameboy->memory.internalRam + bank * MEMORY_MAP_SEGMENT_SIZE :
            gameboy->memory.internalRam + MEMORY_MAP_SEGMENT_SIZE;
        gameboy->memory.memoryMap[0xF] = gameboy->memory.memoryMap[0xD];
    }

    updateToLatestVersion(gameboy);

    return 0;
}

int saveGameboyState(struct GameBoy* gameboy, enum StoredInfoType storeType)
{    
    if (storeType == StoredInfoTypeNone)
    {
        return -1;
    }

    gameboy->settings.timer = gameboy->memory.misc.time;

    if (gSaveTypeSetting.saveType == SaveTypeFlash && osFlashAllErase()) {
        DEBUG_PRINT_F("Save fail clear\n");
        return -1;
    }

    if (storeType == StoredInfoTypeRAM)
    {
        int sectionSize = RAM_BANK_SIZE * getRAMBankCount(gameboy->memory.rom);
        if (gSaveWriteCallback(gameboy->memory.cartRam, 0, sectionSize)) {
            DEBUG_PRINT_F("Save fail 1 %X $X\n", 0, sectionSize);
            return -1;
        }
    }
    else
    {
        int offset = 0;
        int sectionSize = sizeof(struct GameboySettings);
        if (gSaveWriteCallback(&gameboy->settings, offset, sectionSize)) {
            DEBUG_PRINT_F("Save fail 0 %X $X\n", offset, sectionSize);
            return -1;
        }
        offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

        if (storeType == StoredInfoTypeSettings)
        {
            return 0;
        }
        
        sectionSize = RAM_BANK_SIZE * getRAMBankCount(gameboy->memory.rom);
        if (gSaveWriteCallback(gameboy->memory.cartRam, offset, sectionSize)) {
            DEBUG_PRINT_F("Save fail 1 %X $X\n", offset, sectionSize);
            return -1;
        }
        offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

        if (storeType == StoredInfoTypeSettingsRAM)
        {
            return 0;
        }

        sectionSize = sizeof(struct GraphicsMemory);
        if (gSaveWriteCallback(&gameboy->memory.vram, offset, sectionSize)) {
            DEBUG_PRINT_F("Save fail 2 %X $X\n", offset, sectionSize);
            return -1;
        }
        offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

        sectionSize = MAX_RAM_SIZE;
        if (gSaveWriteCallback(gameboy->memory.internalRam, offset, sectionSize)) {
            DEBUG_PRINT_F("Save fail 3 %X $X\n", offset, sectionSize);
            return -1;
        }
        offset = ALIGN_FLASH_OFFSET(offset + sectionSize);
        
        sectionSize = sizeof(struct MiscMemory);
        if (gSaveWriteCallback(&gameboy->memory.misc, offset, sectionSize)) {
            DEBUG_PRINT_F("Save fail 4 %X $X\n", offset, sectionSize);
            return -1;
        }
        offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

        struct MiscSaveStateData miscData;
        miscData.cpu = gameboy->cpu;
        miscData.audioState = gameboy->memory.audio;
        sectionSize = sizeof(struct MiscSaveStateData);
        if (gSaveWriteCallback(&miscData, offset, sectionSize)) {
            DEBUG_PRINT_F("Save fail 5 %X $X\n", offset, sectionSize);
            return -1;
        }
        offset = ALIGN_FLASH_OFFSET(offset + sectionSize);
    }

    return 0;
}

int getSaveStateSize(struct GameBoy* gameboy)
{
    int offset = 0;
    int sectionSize = sizeof(struct GameboySettings);
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);
    sectionSize = RAM_BANK_SIZE * getRAMBankCount(gameboy->memory.rom);
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);
    sectionSize = sizeof(struct GraphicsMemory);
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);
    sectionSize = MAX_RAM_SIZE;
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);
    sectionSize = sizeof(struct MiscMemory);
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);
    sectionSize = sizeof(struct MiscSaveStateData);
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);
    return offset;
}


void initSaveCallbacks()
{
    if (gSaveTypeSetting.saveType == SAVE_HEADER_VALUE)
    {
        switch (gSaveTypeSetting.saveType)
        {
            case SaveTypeSRAM:
            case SaveTypeSRAM3X:
                gSaveWriteCallback = saveToSRAM;
                gSaveReadCallback = loadFromSRAM;
                break;
            default:
                gSaveWriteCallback = saveToFlash;
                gSaveReadCallback = loadFromFlash;
                osFlashInit();
        }
    }
    else
    {
        gSaveWriteCallback = saveToFlash;
        gSaveReadCallback = loadFromFlash;
        osFlashInit();
    }
}

int getSaveTypeSize(enum SaveType type)
{
    switch (type)
    {
        case SaveTypeFlash:
            return 128 * 1024;
        case SaveTypeSRAM:
            return 32 * 1024;
        case SaveTypeSRAM3X:
            return 3 * 32 * 1024;
    }

    return 0;
}

enum StoredInfoType getStoredInfoType(struct GameBoy* gameboy)
{
    int saveTypeSize = getSaveTypeSize(gSaveTypeSetting.saveType);
    int ramSize = RAM_BANK_SIZE * getRAMBankCount(gameboy->memory.rom) <= saveTypeSize;

    if (getSaveStateSize(gameboy) <= saveTypeSize)
    {
        return StoredInfoTypeAll;
    }
    else if (ramSize + ALIGN_FLASH_OFFSET(sizeof(struct GameboySettings)) <= saveTypeSize)
    {
        return StoredInfoTypeSettingsRAM;
    }
    else if (ramSize <= saveTypeSize)
    {
        return StoredInfoTypeRAM;
    }
    else if (ramSize <= ALIGN_FLASH_OFFSET(sizeof(struct GameboySettings)))
    {
        return StoredInfoTypeSettings;
    }
    else
    {
        return StoredInfoTypeNone;
    }
}