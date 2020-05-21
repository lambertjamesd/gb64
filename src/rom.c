
#include <ultra64.h>

#include "../memory.h"
#include "rom.h"

extern OSMesgQueue     dmaMessageQ;
extern OSMesg          dmaMessageBuf;
extern OSPiHandle	   *handler;
extern OSIoMesg        dmaIOMessageBuf;

struct ROMLayout gGBRom;

#define EXTENDED_BANK_OFFSET    0x8
#define EXTENDED_BANK_ID        0x52

int _romBankSizes[] = {
    1,
    4,
    8,
    16,
    32,
    64,
    128,
    256,
    // extended bank starts here
    72,
    80,
    96,
};

void loadRomSegment(void* target, void *romLocation, int bankNumber)
{
    dmaIOMessageBuf.hdr.pri = OS_MESG_PRI_HIGH;
    dmaIOMessageBuf.hdr.retQueue = &dmaMessageQ;
    dmaIOMessageBuf.dramAddr = target;
    dmaIOMessageBuf.devAddr = (u32)romLocation + bankNumber * ROM_BANK_SIZE;

    osEPiStartDma(handler, &dmaIOMessageBuf, OS_READ);

	/*
	 * Wait for DMA to finish
	 */
	(void) osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
}

void initRomLayout(struct ROMLayout* romLayout, void *romLocation)
{
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
        currentBank = malloc(sizeof(struct VirtualBank));

        if (currentBank)
        {
            currentBank->nextBank = 0;

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
    if (romLayout->mainBank[GB_ROM_H_RAM_SIZE] == 0x3)
    {
        return 4;
    }
    else
    {
        return 1;
    }
}

int getCartType(struct ROMLayout* romLayout)
{
    return romLayout->mainBank[GB_ROM_H_CART_TYPE];
}