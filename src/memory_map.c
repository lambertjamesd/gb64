
#include "memory_map.h"
#include "../memory.h"
#include "debug_out.h"
#include "gameboy.h"
#include "memory_map_offsets.h"

u16 paletteColors[] = {
    0x09C2,
    0x330C,
    0x8D42,
    0x9DC2,
};

u32 gGeneratedReads[8 * MEMORY_MAP_SIZE];
u32 gGeneratedWrites[8 * MEMORY_MAP_SIZE];

void setRomMemoryBank(struct Memory* memory, int offset, void* addr);
void setRamMemoryBank(struct Memory* memory, int offset, void* addr);

void GB_DO_READ_FF();
void GB_DO_WRITE_FF();
void GB_WRITE_ROM_BANK();
void GB_DO_WRITE_NOP();
void GB_DO_READ_NOP();
void GB_DO_READ_MBC2();

void* generateDirectRead(int bank, void* baseAddr)
{
    // offset pointer so adding a GB address points
    // to the correct place in memory
    baseAddr = (char*)baseAddr - bank * 0x1000;

    u32 loAddr = (u32)baseAddr & 0xFFFF;
    u32 hiAddr = ((u32)baseAddr >> 16) + ((loAddr & 0x8000) ? 1 : 0);

    u32* bankRead = gGeneratedReads + 8 * bank;
    u32* bankNoCache = (u32*)K0_TO_K1(bankRead);
    // lui $at, %hi(baseAddr)
    bankNoCache[0] = 0x3C010000 | hiAddr;
    // add $at, $t4, $at
    bankNoCache[1] = 0x01810820;
    // jr $ra
    bankNoCache[2] = 0x03E00008;
    // lbu $v0, %lo(baseAddr)($at)
    bankNoCache[3] = 0x90220000 | loAddr;
    osInvalICache(bankRead, 8 * sizeof(u32));

    return bankRead;
}

void* generateDirectWrite(int bank, void* baseAddr)
{
    // offset pointer so adding a GB address points
    // to the correct place in memory
    baseAddr = (char*)baseAddr - bank * 0x1000;

    u32 loAddr = (u32)baseAddr & 0xFFFF;
    u32 hiAddr = ((u32)baseAddr >> 16) + ((loAddr & 0x8000) ? 1 : 0);

    u32* bankRead = gGeneratedWrites + 8 * bank;
    u32* bankNoCache = (u32*)K0_TO_K1(bankRead);
    // lui $at, %hi(baseAddr)
    bankNoCache[0] = 0x3C010000 | hiAddr;
    // add $at, $t4, $at
    bankNoCache[1] = 0x01810820;
    // jr $ra
    bankNoCache[2] = 0x03E00008;
    // sb $v0, %lo(baseAddr)($at)
    bankNoCache[3] = 0xA02D0000 | loAddr;
    osInvalICache(bankRead, 8 * sizeof(u32));

    return bankRead;
}

// background color? 0xCADC9F

void nopBankSwitch(struct Memory* memory, int addr, int value)
{

}

void handleMBC1Write(struct Memory* memory, int addr, int value)
{
    int writeRange = addr >> 13;
    if (writeRange == 0)
    {
        memory->misc.ramDisabled = (value & 0xA) != 0xA;
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

    setRomMemoryBank(memory, 0x4, romBank + MEMORY_MAP_SEGMENT_SIZE * 0);
    setRomMemoryBank(memory, 0x5, romBank + MEMORY_MAP_SEGMENT_SIZE * 1);
    setRomMemoryBank(memory, 0x6, romBank + MEMORY_MAP_SEGMENT_SIZE * 2);
    setRomMemoryBank(memory, 0x7, romBank + MEMORY_MAP_SEGMENT_SIZE * 3);
    
    if (memory->misc.ramDisabled)
    {
        setMemoryBank(memory, 0xA, ramBank + MEMORY_MAP_SEGMENT_SIZE * 0, &GB_DO_READ_NOP, &GB_DO_WRITE_NOP);
        setMemoryBank(memory, 0xB, ramBank + MEMORY_MAP_SEGMENT_SIZE * 1, &GB_DO_READ_NOP, &GB_DO_WRITE_NOP);
    }
    else
    {
        setRamMemoryBank(memory, 0xA, ramBank + MEMORY_MAP_SEGMENT_SIZE * 0);
        setRamMemoryBank(memory, 0xB, ramBank + MEMORY_MAP_SEGMENT_SIZE * 1);
    }
}

extern void mbc2ReadRam();

void handleMBC2Write(struct Memory* memory, int addr, int value)
{
    int writeRange = addr >> 13;
    if (writeRange == 0)
    {
        memory->misc.ramDisabled = (value & 0xA) != 0xA;
    }
    else if (writeRange == 1)
    {  
        memory->misc.romBankLower = value;
    }

    char* romBank;
    int bankIndex = memory->misc.romBankLower & 0xF;

    romBank = getROMBank(memory->rom, bankIndex ? bankIndex : 1);

    setRomMemoryBank(memory, 0x4, romBank + MEMORY_MAP_SEGMENT_SIZE * 0);
    setRomMemoryBank(memory, 0x5, romBank + MEMORY_MAP_SEGMENT_SIZE * 1);
    setRomMemoryBank(memory, 0x6, romBank + MEMORY_MAP_SEGMENT_SIZE * 2);
    setRomMemoryBank(memory, 0x7, romBank + MEMORY_MAP_SEGMENT_SIZE * 3);

    if (memory->misc.ramDisabled)
    {
        setMemoryBank(memory, 0xA, memory->cartRam + MEMORY_MAP_SEGMENT_SIZE * 0, &GB_DO_READ_NOP, &GB_DO_WRITE_NOP);
        setMemoryBank(memory, 0xB, memory->cartRam + MEMORY_MAP_SEGMENT_SIZE * 1, &GB_DO_READ_NOP, &GB_DO_WRITE_NOP);
    }
    else
    {
        setMemoryBank(memory, 0xA, memory->cartRam, &GB_DO_READ_MBC2, generateDirectWrite(0xA, memory->cartRam));
        setMemoryBank(memory, 0xB, memory->cartRam + MEMORY_MAP_SEGMENT_SIZE * 1, &GB_DO_READ_NOP, &GB_DO_WRITE_NOP);
    }
}


void handleMMM0Write(struct Memory* memory, int addr, int value)
{
    // TODO
}

extern void mbc3WriteTimer();
extern void mbc3ReadTimer();

void writeMBC3ClockRegisters(u64 time, u8* target)
{
    time /= CPU_TICKS_PER_SECOND;
    target[REG_RTC_S - REG_RTC_S] = time % 60;
    time /= 60;
    target[REG_RTC_M - REG_RTC_S] = time % 60;
    time /= 60;
    target[REG_RTC_H - REG_RTC_S] = time % 24;
    time /= 24;
    target[REG_RTC_DL - REG_RTC_S] = time & 0xFF;
    time /= 0x100;
    
    u8 rtcDH = target[REG_RTC_DH - REG_RTC_S];
    rtcDH &= REG_RTC_DH_HALT;

    rtcDH |= time & REG_RTC_DH_HIGH;
    rtcDH |= (time > 0x1) ? REG_RTC_DH_C : 0;
    target[REG_RTC_DH - REG_RTC_S] = rtcDH;
}

u64 readMBC3ClockRegisters(u8* source)
{
    u64 result = ((source[REG_RTC_DH - REG_RTC_S] & 0x1) << 8) |
        source[REG_RTC_DL - REG_RTC_S];

    result = result * 24 + source[REG_RTC_H - REG_RTC_S];
    result = result * 60 + source[REG_RTC_M - REG_RTC_S];
    result = result * 60 + source[REG_RTC_S - REG_RTC_S];

    return result * CPU_TICKS_PER_SECOND;
}

void handleMBC3Write(struct Memory* memory, int addr, int value)
{
    int writeRange = addr >> 13;
    if (writeRange == 0)
    {
        memory->misc.ramDisabled = (value & 0xA) != 0xA;
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
            writeMBC3ClockRegisters(memory->misc.time, &READ_REGISTER_DIRECT(memory, REG_RTC_S));
        }
        
        memory->misc.romBankUpper = value;
    }

    if (memory->misc.ramDisabled)
    {
        setMemoryBank(memory, 0xA, memory->cartRam + MEMORY_MAP_SEGMENT_SIZE * 0, &GB_DO_READ_NOP, &GB_DO_WRITE_NOP);
        setMemoryBank(memory, 0xB, memory->cartRam + MEMORY_MAP_SEGMENT_SIZE * 1, &GB_DO_READ_NOP, &GB_DO_WRITE_NOP);
    }
    else if (memory->misc.ramRomSelect < 4)
    {
        char* ramBank = memory->cartRam + (memory->misc.ramRomSelect & 0x3) * MEMORY_MAP_SEGMENT_SIZE * 2;
        setRamMemoryBank(memory, 0xA, ramBank + MEMORY_MAP_SEGMENT_SIZE * 0);
        setRamMemoryBank(memory, 0xB, ramBank + MEMORY_MAP_SEGMENT_SIZE * 1);
    } 
    else
    {
        char* ramBank = memory->cartRam;
        setMemoryBank(memory, 0xA, ramBank + MEMORY_MAP_SEGMENT_SIZE * 0, mbc3ReadTimer, mbc3WriteTimer);
        setMemoryBank(memory, 0xB, ramBank + MEMORY_MAP_SEGMENT_SIZE * 1, mbc3ReadTimer, mbc3WriteTimer);
    }

    char* romBank = getROMBank(memory->rom, (memory->misc.romBankLower & 0x7F) ? memory->misc.romBankLower & 0x7F : 1);

    setRomMemoryBank(memory, 0x4, romBank + MEMORY_MAP_SEGMENT_SIZE * 0);
    setRomMemoryBank(memory, 0x5, romBank + MEMORY_MAP_SEGMENT_SIZE * 1);
    setRomMemoryBank(memory, 0x6, romBank + MEMORY_MAP_SEGMENT_SIZE * 2);
    setRomMemoryBank(memory, 0x7, romBank + MEMORY_MAP_SEGMENT_SIZE * 3);
}


void handleMBC5Write(struct Memory* memory, int addr, int value)
{
    int writeRange = addr >> 12;
    switch (addr >> 12) {
        case 0: case 1:
            memory->misc.ramDisabled = (value & 0xA) != 0xA;
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

    setRomMemoryBank(memory, 0x4, romBank + MEMORY_MAP_SEGMENT_SIZE * 0);
    setRomMemoryBank(memory, 0x5, romBank + MEMORY_MAP_SEGMENT_SIZE * 1);
    setRomMemoryBank(memory, 0x6, romBank + MEMORY_MAP_SEGMENT_SIZE * 2);
    setRomMemoryBank(memory, 0x7, romBank + MEMORY_MAP_SEGMENT_SIZE * 3);
    
    if (memory->misc.ramDisabled)
    {
        setMemoryBank(memory, 0xA, memory->cartRam + MEMORY_MAP_SEGMENT_SIZE * 0, &GB_DO_READ_NOP, &GB_DO_WRITE_NOP);
        setMemoryBank(memory, 0xB, memory->cartRam + MEMORY_MAP_SEGMENT_SIZE * 1, &GB_DO_READ_NOP, &GB_DO_WRITE_NOP);
    }
    else
    {
        setRamMemoryBank(memory, 0xA, ramBank + MEMORY_MAP_SEGMENT_SIZE * 0);
        setRamMemoryBank(memory, 0xB, ramBank + MEMORY_MAP_SEGMENT_SIZE * 1);
    }
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

void setMemoryBank(struct Memory* memory, int offset, void* addr, void* readCallback, void* writeCallback) 
{
    memory->memoryMap[offset] = addr;
    memory->cartMemoryRead[offset] = readCallback;
    memory->cartMemoryWrite[offset] = writeCallback;
}

void setRomMemoryBank(struct Memory* memory, int offset, void* addr) {
    setMemoryBank(memory, offset, addr, generateDirectRead(offset, addr), &GB_WRITE_ROM_BANK);
}

void setRamMemoryBank(struct Memory* memory, int offset, void* addr) {
    setMemoryBank(memory, offset, addr, generateDirectRead(offset, addr), generateDirectWrite(offset, addr));
}

void setVRAMBank(struct Memory* memory, int value) {
    int bankOffset = (int)memory->vramBytes;

    if (value == 1) {
        bankOffset += MEMORY_MAP_SEGMENT_SIZE * 2;
    }

    setRamMemoryBank(memory, 0x8, (void*)(bankOffset + MEMORY_MAP_SEGMENT_SIZE * 0));
    setRamMemoryBank(memory, 0x9, (void*)(bankOffset + MEMORY_MAP_SEGMENT_SIZE * 1));
}

void setInternalRamBank(struct Memory* memory, int value) {
    if (value < 1) {
        value = 1;
    } else if (value > 7) {
        value = 7;
    }

    int bankOffset = (int)memory->internalRam + value * MEMORY_MAP_SEGMENT_SIZE;
    setRamMemoryBank(memory, 0xD, (void*)bankOffset);
    setMemoryBank(memory, 0xF, (void*)bankOffset, &GB_DO_READ_FF, &GB_DO_WRITE_FF);
}

void* getMemoryBank(struct Memory* memory, int offset)
{
    return memory->memoryMap[offset];
}

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
    memory->misc.ramDisabled = 1;
    
    finishRomLoad(rom);

    setRomMemoryBank(memory, 0x0, rom->mainBank + MEMORY_MAP_SEGMENT_SIZE * 0);
    setRomMemoryBank(memory, 0x1, rom->mainBank + MEMORY_MAP_SEGMENT_SIZE * 1);
    setRomMemoryBank(memory, 0x2, rom->mainBank + MEMORY_MAP_SEGMENT_SIZE * 2);
    setRomMemoryBank(memory, 0x3, rom->mainBank + MEMORY_MAP_SEGMENT_SIZE * 3);
    
    memoryBank = getROMBank(rom, 1);
    
    setRomMemoryBank(memory, 0x4, memoryBank + MEMORY_MAP_SEGMENT_SIZE * 0);
    setRomMemoryBank(memory, 0x5, memoryBank + MEMORY_MAP_SEGMENT_SIZE * 1);
    setRomMemoryBank(memory, 0x6, memoryBank + MEMORY_MAP_SEGMENT_SIZE * 2);
    setRomMemoryBank(memory, 0x7, memoryBank + MEMORY_MAP_SEGMENT_SIZE * 3);
    
    setRamMemoryBank(memory, 0x8, memory->vramBytes + MEMORY_MAP_SEGMENT_SIZE * 0);
    setRamMemoryBank(memory, 0x9, memory->vramBytes + MEMORY_MAP_SEGMENT_SIZE * 1);

    setMemoryBank(memory, 0xA, memory->cartRam + MEMORY_MAP_SEGMENT_SIZE * 0, &GB_DO_READ_NOP, &GB_DO_WRITE_NOP);
    setMemoryBank(memory, 0xB, memory->cartRam + MEMORY_MAP_SEGMENT_SIZE * 1, &GB_DO_READ_NOP, &GB_DO_WRITE_NOP);
    
    setRamMemoryBank(memory, 0xC, memory->internalRam + MEMORY_MAP_SEGMENT_SIZE * 0);
    setRamMemoryBank(memory, 0xD, memory->internalRam + MEMORY_MAP_SEGMENT_SIZE * 1);
    
    setRamMemoryBank(memory, 0xE, memory->internalRam + MEMORY_MAP_SEGMENT_SIZE * 0);
    setMemoryBank(memory, 0xF, memory->internalRam + MEMORY_MAP_SEGMENT_SIZE * 1, &GB_DO_READ_FF, &GB_DO_WRITE_FF);

    WRITE_REGISTER_DIRECT(memory, REG_INT_REQUESTED, 0xE0);
    WRITE_REGISTER_DIRECT(memory, REG_NR52, 0xF0);
    WRITE_REGISTER_DIRECT(memory, REG_UNLOAD_BIOS, 0x00);
    WRITE_REGISTER_DIRECT(memory, REG_HDMA5, 0xFF);
}