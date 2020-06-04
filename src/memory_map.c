
#include "memory_map.h"
#include "../memory.h"

u16 palleteColors[] = {
    0x09C2,
    0x330C,
    0x8D42,
    0x9DC2,
};

// background color? 0xCADC9F

void nopBankSwitch(struct Memory* memory, int addr, unsigned char value)
{

}

void handleMBC1Write(struct Memory* memory, int addr, unsigned char value)
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
    
    if (memory->misc.ramRomSelect)
    {
        ramBank = memory->cartRam + (memory->misc.romBankUpper & 0x3) * MEMORY_MAP_SEGMENT_SIZE * 2;
        romBank = getROMBank(memory->rom, memory->misc.romBankLower & 0x1F);
    }
    else
    {
        ramBank = memory->cartRam;
        romBank = getROMBank(memory->rom, (memory->misc.romBankLower & 0x1F) | ((memory->misc.romBankUpper & 0x3) << 5));
    }

    memory->memoryMap[0x4] = romBank + MEMORY_MAP_SEGMENT_SIZE * 0;
    memory->memoryMap[0x5] = romBank + MEMORY_MAP_SEGMENT_SIZE * 1;
    memory->memoryMap[0x6] = romBank + MEMORY_MAP_SEGMENT_SIZE * 2;
    memory->memoryMap[0x7] = romBank + MEMORY_MAP_SEGMENT_SIZE * 3;
    
    memory->memoryMap[0xA] = ramBank + MEMORY_MAP_SEGMENT_SIZE * 0;
    memory->memoryMap[0xB] = ramBank + MEMORY_MAP_SEGMENT_SIZE * 1;
}

void defaultRegisterWrite(struct Memory* memory, int addr, unsigned char value)
{
    WRITE_REGISTER_DIRECT(memory, addr, value);
}

void initMemory(struct Memory* memory, struct ROMLayout* rom)
{
    char *memoryBank;
    int index;

    zeroMemory(memory, sizeof(struct Memory));
    memory->rom = rom;
    memory->cartRam = malloc(RAM_BANK_SIZE * getRAMBankCount(rom));
    
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

    for (index = 0; index < REGISTER_WRITER_COUNT; ++index)
    {
        memory->registerWriters[index] = defaultRegisterWrite;
    }

    
    switch (rom->mainBank[GB_ROM_H_CART_TYPE])
    {
        case 0x01:
        case 0x02:
        case 0x03:
            memory->bankSwitch = handleMBC1Write;
            break;
        default:
        memory->bankSwitch = nopBankSwitch;
    }
}