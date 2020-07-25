
#include "memory_map.h"
#include "../memory.h"
#include "debug_out.h"
#include "gameboy.h"

u16 paletteColors[] = {
    0x09C2,
    0x330C,
    0x8D42,
    0x9DC2,
};

// background color? 0xCADC9F

void nopBankSwitch(struct Memory* memory, int addr, int value)
{

}

void handleMBC1Write(struct Memory* memory, int addr, int value)
{
    int writeRange = addr >> 13;
    if (writeRange == 0)
    {
        // RAM Enable, do nothing for now
    }
    else if (writeRange == 1)
    {  
        memory->misc.romBankLower = value;
    }
    else if (writeRange == 2)
    {
        memory->misc.romBankUpper = value;
    }
    else if (writeRange == 3)
    {
        memory->misc.ramRomSelect = value;
    }

    char* ramBank;
    char* romBank;
    int bankIndex;
    
    if (memory->misc.ramRomSelect)
    {
        bankIndex = memory->misc.romBankLower & 0x1F;
        ramBank = memory->cartRam + (memory->misc.romBankUpper & 0x3) * MEMORY_MAP_SEGMENT_SIZE * 2;
    }
    else
    {
        bankIndex = (memory->misc.romBankLower & 0x1F) | ((memory->misc.romBankUpper & 0x3) << 5);
        ramBank = memory->cartRam;
    }
    romBank = getROMBank(memory->rom, bankIndex ? bankIndex : 1);

    memory->memoryMap[0x4] = romBank + MEMORY_MAP_SEGMENT_SIZE * 0;
    memory->memoryMap[0x5] = romBank + MEMORY_MAP_SEGMENT_SIZE * 1;
    memory->memoryMap[0x6] = romBank + MEMORY_MAP_SEGMENT_SIZE * 2;
    memory->memoryMap[0x7] = romBank + MEMORY_MAP_SEGMENT_SIZE * 3;
    
    memory->memoryMap[0xA] = ramBank + MEMORY_MAP_SEGMENT_SIZE * 0;
    memory->memoryMap[0xB] = ramBank + MEMORY_MAP_SEGMENT_SIZE * 1;
}

extern void mbc2ReadRam();

void handleMBC2Write(struct Memory* memory, int addr, int value)
{
    int writeRange = addr >> 13;
    if (writeRange == 0)
    {
        // RAM Enable, do nothing for now
    }
    else if (writeRange == 1)
    {  
        memory->misc.romBankLower = value;
    }

    char* romBank;
    int bankIndex = memory->misc.romBankLower & 0xF;

    romBank = getROMBank(memory->rom, bankIndex ? bankIndex : 1);

    memory->memoryMap[0x4] = romBank + MEMORY_MAP_SEGMENT_SIZE * 0;
    memory->memoryMap[0x5] = romBank + MEMORY_MAP_SEGMENT_SIZE * 1;
    memory->memoryMap[0x6] = romBank + MEMORY_MAP_SEGMENT_SIZE * 2;
    memory->memoryMap[0x7] = romBank + MEMORY_MAP_SEGMENT_SIZE * 3;

    memory->cartRamRead = mbc2ReadRam;
}


void handleMMM0Write(struct Memory* memory, int addr, int value)
{
    // TODO
}

extern void mbc3WriteTimer();
extern void mbc3ReadTimer();

void handleMBC3Write(struct Memory* memory, int addr, int value)
{
    int writeRange = addr >> 13;
    if (writeRange == 0)
    {
        // Do nothing
    }
    else if (writeRange == 1)
    {  
        memory->misc.romBankLower = value;
    }
    else if (writeRange == 2)
    {
        memory->misc.ramRomSelect = value;
    }
    else if (writeRange == 3)
    {
        if (memory->misc.romBankUpper == 0 && value == 1)
        {
            u64 allTime = memory->misc.time;
            allTime /= CPU_TICKS_PER_SECOND;
            WRITE_REGISTER_DIRECT(memory, REG_RTC_S, allTime % 60);
            allTime /= 60;
            WRITE_REGISTER_DIRECT(memory, REG_RTC_M, allTime % 60);
            allTime /= 60;
            WRITE_REGISTER_DIRECT(memory, REG_RTC_H, allTime % 24);
            allTime /= 24;
            WRITE_REGISTER_DIRECT(memory, REG_RTC_DL, allTime & 0xFF);
            allTime /= 0x100;
            
            u8 rtcDH = READ_REGISTER_DIRECT(memory, REG_RTC_DH);
            rtcDH &= REG_RTC_DH_HALT;

            rtcDH |= allTime & REG_RTC_DH_HIGH;
            rtcDH |= (allTime > 0x1) ? REG_RTC_DH_C : 0;
            WRITE_REGISTER_DIRECT(memory, REG_RTC_DH, rtcDH);
        }
        memory->misc.romBankUpper = value;
    }

    char* ramBank;
    if (memory->misc.ramRomSelect < 4) {
        ramBank = memory->cartRam + (memory->misc.ramRomSelect & 0x3) * MEMORY_MAP_SEGMENT_SIZE * 2;
        memory->cartRamRead = NULL;
        memory->cartRamWrite = NULL;
    } else {
        ramBank = memory->cartRam;
        memory->cartRamRead = mbc3ReadTimer;
        memory->cartRamWrite = mbc3WriteTimer;
    }

    char* romBank = getROMBank(memory->rom, (memory->misc.romBankLower & 0x7F) ? memory->misc.romBankLower & 0x7F : 1);

    memory->memoryMap[0x4] = romBank + MEMORY_MAP_SEGMENT_SIZE * 0;
    memory->memoryMap[0x5] = romBank + MEMORY_MAP_SEGMENT_SIZE * 1;
    memory->memoryMap[0x6] = romBank + MEMORY_MAP_SEGMENT_SIZE * 2;
    memory->memoryMap[0x7] = romBank + MEMORY_MAP_SEGMENT_SIZE * 3;
    
    memory->memoryMap[0xA] = ramBank + MEMORY_MAP_SEGMENT_SIZE * 0;
    memory->memoryMap[0xB] = ramBank + MEMORY_MAP_SEGMENT_SIZE * 1;
}


void handleMBC5Write(struct Memory* memory, int addr, int value)
{
    int writeRange = addr >> 12;
    switch (addr >> 12) {
        case 0: case 1:
            break;
        case 2:
            memory->misc.romBankLower = value;
            break;
        case 3:
            memory->misc.romBankUpper = value;
            break;
        case 4: case 5:
            memory->misc.ramRomSelect = value;
            break;
    }

    char* ramBank = memory->cartRam + (memory->misc.ramRomSelect & 0xF) * MEMORY_MAP_SEGMENT_SIZE * 2;
    char* romBank = getROMBank(memory->rom, memory->misc.romBankLower | (memory->misc.romBankUpper << 8 & 0x100));

    memory->memoryMap[0x4] = romBank + MEMORY_MAP_SEGMENT_SIZE * 0;
    memory->memoryMap[0x5] = romBank + MEMORY_MAP_SEGMENT_SIZE * 1;
    memory->memoryMap[0x6] = romBank + MEMORY_MAP_SEGMENT_SIZE * 2;
    memory->memoryMap[0x7] = romBank + MEMORY_MAP_SEGMENT_SIZE * 3;
    
    memory->memoryMap[0xA] = ramBank + MEMORY_MAP_SEGMENT_SIZE * 0;
    memory->memoryMap[0xB] = ramBank + MEMORY_MAP_SEGMENT_SIZE * 1;
}

void defaultRegisterWrite(struct Memory* memory, int addr, int value)
{
    WRITE_REGISTER_DIRECT(memory, addr, value);
}

struct MBCData mbcTypes[] = {
    {nopBankSwitch, 0x00, 0},
    {handleMBC1Write, 0x01, 0},
    {handleMBC1Write, 0x02, MBC_FLAGS_RAM},
    {handleMBC1Write, 0x03, MBC_FLAGS_RAM | MBC_FLAGS_BATTERY},
    {handleMBC2Write, 0x05, 0},
    {handleMBC2Write, 0x06, MBC_FLAGS_BATTERY},
    {nopBankSwitch, 0x08, MBC_FLAGS_RAM},
    {nopBankSwitch, 0x09, MBC_FLAGS_RAM | MBC_FLAGS_BATTERY},
    {NULL, 0x0B, 0},
    {NULL, 0x0C, MBC_FLAGS_RAM},
    {NULL, 0x0D, MBC_FLAGS_RAM | MBC_FLAGS_BATTERY},
    {handleMBC3Write, 0x0F, MBC_FLAGS_TIMER | MBC_FLAGS_BATTERY},
    {handleMBC3Write, 0x10, MBC_FLAGS_TIMER | MBC_FLAGS_RAM | MBC_FLAGS_BATTERY},
    {handleMBC3Write, 0x11, 0},
    {handleMBC3Write, 0x12, MBC_FLAGS_RAM},
    {handleMBC3Write, 0x13, MBC_FLAGS_RAM | MBC_FLAGS_BATTERY},
    {NULL, 0x15, 0},
    {NULL, 0x16, MBC_FLAGS_RAM},
    {NULL, 0x17, MBC_FLAGS_RAM | MBC_FLAGS_BATTERY},
    {handleMBC5Write, 0x19, 0},
    {handleMBC5Write, 0x1A, MBC_FLAGS_RAM},
    {handleMBC5Write, 0x1B, MBC_FLAGS_RAM | MBC_FLAGS_BATTERY},
    {handleMBC5Write, 0x1C, MBC_FLAGS_RUMBLE},
    {handleMBC5Write, 0x1D, MBC_FLAGS_RUMBLE | MBC_FLAGS_RAM},
    {handleMBC5Write, 0x1E, MBC_FLAGS_RUMBLE | MBC_FLAGS_RAM | MBC_FLAGS_BATTERY},
    {NULL, 0xFC, 0},
    {NULL, 0xFD, 0},
    {NULL, 0xFE, 0},
    {NULL, 0xFF, 0},
};

#define MBC_TYPES_LENGTH (sizeof(mbcTypes) / sizeof(mbcTypes[0]))

void initMemory(struct Memory* memory, struct ROMLayout* rom)
{
    char *memoryBank;
    int index;
    struct MBCData* mbc = NULL;

    for (index = 0; index < MBC_TYPES_LENGTH; ++index)
    {
        if (rom->mainBank[GB_ROM_H_CART_TYPE] == mbcTypes[index].id) {
            mbc = &mbcTypes[index];
            break;
        }
    }

    zeroMemory(memory, sizeof(struct Memory));
    memory->mbc = mbc;
    memory->rom = rom;
    memory->cartRam = malloc(RAM_BANK_SIZE * getRAMBankCount(rom));

    
    if (!mbc) {
        DEBUG_PRINT_F("Bad MBC %X\n", rom->mainBank[GB_ROM_H_CART_TYPE]);
        memory->bankSwitch = nopBankSwitch;
    } else if (!mbc->bankSwitch) {
        DEBUG_PRINT_F("TODO MBC %X\n", mbcTypes[index].id);
        memory->bankSwitch = nopBankSwitch;
    } else {
        memory->bankSwitch = mbc->bankSwitch;
    }

	memory->audio.noiseSound.lfsr = 0x7FFF;
    
    finishRomLoad(rom);

    memory->memoryMap[0x0] = rom->mainBank + MEMORY_MAP_SEGMENT_SIZE * 0;
    memory->memoryMap[0x1] = rom->mainBank + MEMORY_MAP_SEGMENT_SIZE * 1;
    memory->memoryMap[0x2] = rom->mainBank + MEMORY_MAP_SEGMENT_SIZE * 2;
    memory->memoryMap[0x3] = rom->mainBank + MEMORY_MAP_SEGMENT_SIZE * 3;
    
    memoryBank = getROMBank(rom, 1);
    
    memory->memoryMap[0x4] = memoryBank + MEMORY_MAP_SEGMENT_SIZE * 0;
    memory->memoryMap[0x5] = memoryBank + MEMORY_MAP_SEGMENT_SIZE * 1;
    memory->memoryMap[0x6] = memoryBank + MEMORY_MAP_SEGMENT_SIZE * 2;
    memory->memoryMap[0x7] = memoryBank + MEMORY_MAP_SEGMENT_SIZE * 3;
    
    memory->memoryMap[0x8] = memory->vramBytes + MEMORY_MAP_SEGMENT_SIZE * 0;
    memory->memoryMap[0x9] = memory->vramBytes + MEMORY_MAP_SEGMENT_SIZE * 1;

    memory->memoryMap[0xA] = memory->cartRam + MEMORY_MAP_SEGMENT_SIZE * 0;
    memory->memoryMap[0xB] = memory->cartRam + MEMORY_MAP_SEGMENT_SIZE * 1;
    
    memory->memoryMap[0xC] = memory->internalRam + MEMORY_MAP_SEGMENT_SIZE * 0;
    memory->memoryMap[0xD] = memory->internalRam + MEMORY_MAP_SEGMENT_SIZE * 1;
    
    memory->memoryMap[0xE] = memory->internalRam + MEMORY_MAP_SEGMENT_SIZE * 0;
    memory->memoryMap[0xF] = memory->internalRam + MEMORY_MAP_SEGMENT_SIZE * 1;

    WRITE_REGISTER_DIRECT(memory, REG_INT_REQUESTED, 0xE0);
    WRITE_REGISTER_DIRECT(memory, REG_NR52, 0xF0);
    WRITE_REGISTER_DIRECT(memory, REG_UNLOAD_BIOS, 0x00);
    WRITE_REGISTER_DIRECT(memory, REG_HDMA5, 0xFF);
}