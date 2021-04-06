
#include <ultra64.h>

#include "../memory.h"
#include "rom.h"
#include "debug_out.h"

extern OSMesgQueue     dmaMessageQ;
extern OSMesg          dmaMessageBuf;
extern OSPiHandle	   *handler;
extern OSIoMesg        dmaIOMessageBuf;

struct ROMLayout gGBRom;

#define EXTENDED_BANK_OFFSET    0x9
#define EXTENDED_BANK_ID        0x52

// preserve 256k of ram
#define RESERVE_RAM             256 * 1024

int _romBankSizes[] = {
    2,
    4,
    8,
    16,
    32,
    64,
    128,
    256,
    512,
    // extended bank starts here
    72,
    80,
    96,
};

void loadRomSegment(void* target, void *romLocation, int bankNumber)
{
	OSIoMesg dmaIoMesgBuf;

    dmaIoMesgBuf.hdr.pri = OS_MESG_PRI_HIGH;
    dmaIoMesgBuf.hdr.retQueue = &dmaMessageQ;
    dmaIoMesgBuf.dramAddr = target;
    dmaIoMesgBuf.devAddr = (u32)romLocation + bankNumber * ROM_BANK_SIZE;
    dmaIoMesgBuf.size = ROM_BANK_SIZE;

    osEPiStartDma(handler, &dmaIoMesgBuf, OS_READ);
	(void) osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
    osInvalDCache(target, ROM_BANK_SIZE);
}

void initRomLayout(struct ROMLayout* romLayout, void *romLocation)
{
    // allocate memory for the first and second bank
    romLayout->mainBank = malloc(ROM_BANK_SIZE + sizeof(struct VirtualBank));

    loadRomSegment(romLayout->mainBank, romLocation, 0);
    romLayout->firstVirtualBank = 0;
    romLayout->lastVirtualBank = 0;
    romLayout->romBankToVirtualBank = 0;
    romLayout->romBankCount = 0;
    romLayout->romLocation = romLocation;
}

void finishRomLoad(struct ROMLayout* romLayout)
{
    int bankIndex;
    struct VirtualBank* currentBank;

    romLayout->romBankCount = getROMBankCount(romLayout);
    romLayout->romBankToVirtualBank = malloc(sizeof(struct VirtualBank*) * romLayout->romBankCount);

    for (bankIndex = 1; bankIndex < romLayout->romBankCount; ++bankIndex)
    {
        if (bankIndex == 1) {
            // the fist bank is a special case. As an optimization for decoding instructions
            // a pointer is maintained pointing to the current instruction. Some games will
            // spill over from bank 0 to bank 1. By ensuring the banks are contigious in memory
            // these games will still work assuming the spill over doesn't happen after the bank
            // has been switched
            currentBank = (struct VirtualBank*)(romLayout->mainBank + ROM_BANK_SIZE);
        } else if (calculateBytesFree() - sizeof(struct VirtualBank) > RESERVE_RAM) {
            currentBank = malloc(sizeof(struct VirtualBank));
        } else {
            currentBank = 0;
        }

        if (currentBank)
        {
            currentBank->nextBank = 0;
            currentBank->bankIndex = bankIndex;

            if (!romLayout->firstVirtualBank)
            {
                romLayout->firstVirtualBank = currentBank;
            }

            if (romLayout->lastVirtualBank)
            {
                currentBank->prevBank = romLayout->lastVirtualBank;
                romLayout->lastVirtualBank->nextBank = currentBank;
            }
            else
            {
                currentBank->prevBank = 0;
            }

            romLayout->lastVirtualBank = currentBank;

            loadRomSegment(currentBank->bankMemory, romLayout->romLocation, bankIndex);
        }

        romLayout->romBankToVirtualBank[bankIndex-1] = currentBank;
    }

    if (bankIndex-1 == romLayout->lastVirtualBank->bankIndex)
    {
        // if all banks were loaded set firstVirtualBank to null
        // to indicate that there is no need to load from
        // the rom
        romLayout->firstVirtualBank = NULL;
        romLayout->lastVirtualBank = NULL;
    }
}

char* getROMBank(struct ROMLayout* romLayout, int bankIndex)
{
    struct VirtualBank *useBank;

    if (bankIndex <= 0)
    {
        return romLayout->mainBank;
    }
    else if (bankIndex >= romLayout->romBankCount)
    {
        bankIndex = bankIndex % romLayout->romBankCount;
    }

    useBank = romLayout->romBankToVirtualBank[bankIndex-1];

    if (romLayout->firstVirtualBank && romLayout->lastVirtualBank)
    {
        if (!useBank)
        {
            // if no memory bank is available grab the last used bank
            useBank = romLayout->lastVirtualBank;
            romLayout->romBankToVirtualBank[useBank->bankIndex-1] = 0;
            romLayout->romBankToVirtualBank[bankIndex-1] = useBank;
            useBank->bankIndex = bankIndex;
            loadRomSegment(useBank->bankMemory, romLayout->romLocation, bankIndex);
        }

        // move the bank to the front of the queue
        if (useBank != romLayout->firstVirtualBank)
        {
            if (useBank == romLayout->lastVirtualBank)
            {
                romLayout->lastVirtualBank = useBank->prevBank;
            }

            // remove from doubly linked list
            if (useBank->nextBank)
            {
                useBank->nextBank->prevBank = useBank->prevBank;
            }
            useBank->prevBank->nextBank = useBank->nextBank;

            romLayout->firstVirtualBank->prevBank = useBank;
            useBank->nextBank = romLayout->firstVirtualBank;
            romLayout->firstVirtualBank = useBank;
        }
    }

    return useBank->bankMemory;
}

int getROMBankCount(struct ROMLayout* romLayout)
{
    unsigned char bankType;
    bankType = romLayout->mainBank[GB_ROM_H_SIZE];

    if (bankType < EXTENDED_BANK_OFFSET)
    {
        return _romBankSizes[bankType];
    }
    else if (bankType >= EXTENDED_BANK_ID)
    {
        return _romBankSizes[bankType - EXTENDED_BANK_ID + EXTENDED_BANK_OFFSET];
    }
    else
    {
        return 1;
    }
}

int getRAMBankCount(struct ROMLayout* romLayout)
{
    switch (romLayout->mainBank[GB_ROM_H_RAM_SIZE])
    { 
        case 0x0:
        case 0x1:
        case 0x2:
            return 1;
        case 0x3:
            return 4;
        case 0x4:
            return 16;
        case 0x5:
            return 8;
        default:
            DEBUG_PRINT_F("Invalid ram size");
            return 1;
    }
}

int getCartType(struct ROMLayout* romLayout)
{
    return romLayout->mainBank[GB_ROM_H_CART_TYPE];
}

void loadBIOS(struct ROMLayout* romLayout, int gbc)
{
    if (gbc) 
    {
        OSIoMesg dmaIoMesgBuf;

        dmaIoMesgBuf.hdr.pri = OS_MESG_PRI_HIGH;
        dmaIoMesgBuf.hdr.retQueue = &dmaMessageQ;
        dmaIoMesgBuf.dramAddr = romLayout->mainBank;
        dmaIoMesgBuf.devAddr = (u32)_cgb_biosSegmentRomStart;
        dmaIoMesgBuf.size = 0x100;

        osEPiStartDma(handler, &dmaIoMesgBuf, OS_READ);
        (void) osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);

        // split into two dmas since the gbc bios doesn't overwrite addresses 0x100-0x1FF
        
        dmaIoMesgBuf.hdr.pri = OS_MESG_PRI_HIGH;
        dmaIoMesgBuf.hdr.retQueue = &dmaMessageQ;
        dmaIoMesgBuf.dramAddr = romLayout->mainBank + 0x200;
        dmaIoMesgBuf.devAddr = (u32)_cgb_biosSegmentRomStart + 0x200;
        dmaIoMesgBuf.size = (u32)_cgb_biosSegmentRomEnd - (u32)_cgb_biosSegmentRomStart - 0x200;

        osEPiStartDma(handler, &dmaIoMesgBuf, OS_READ);
        (void) osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
        osInvalDCache(romLayout->mainBank, ROM_BANK_SIZE);
    }
    else
    {
        OSIoMesg dmaIoMesgBuf;

        dmaIoMesgBuf.hdr.pri = OS_MESG_PRI_HIGH;
        dmaIoMesgBuf.hdr.retQueue = &dmaMessageQ;
        dmaIoMesgBuf.dramAddr = romLayout->mainBank;
        dmaIoMesgBuf.devAddr = (u32)_dmg_bootSegmentRomStart;
        dmaIoMesgBuf.size = (u32)_dmg_bootSegmentRomEnd - (u32)_dmg_bootSegmentRomStart;

        osEPiStartDma(handler, &dmaIoMesgBuf, OS_READ);
        (void) osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
        osInvalDCache(romLayout->mainBank, ROM_BANK_SIZE);
    }
}