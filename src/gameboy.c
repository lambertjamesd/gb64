
#include "gameboy.h"
#include "graphics.h"
#include "debugger.h"
#include "debug_out.h"

struct GameBoy gGameboy;

extern OSMesgQueue     dmaMessageQ;
extern OSMesg          dmaMessageBuf;
extern OSPiHandle	   *handler;
extern OSIoMesg        dmaIOMessageBuf;

#define FLASH_BLOCK_SIZE    0x80

#define ALIGN_FLASH_OFFSET(offset) (((offset) + 0x7F) & ~0x7F)

int loadFromFlash(void* target, int sramOffset, int length)
{
    OSIoMesg dmaIoMesgBuf;
    osInvalDCache(target, length);
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

    return 0;
}

int saveToFlash(void *from, int sramOffset, int length)
{
    while (length > 0)
    {
        OSIoMesg dmaIoMesgBuf;
        int pageNumber = sramOffset / FLASH_BLOCK_SIZE;

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

void loadRAM(struct Memory* memory)
{
    if (memory->mbc && memory->mbc->flags | MBC_FLAGS_BATTERY)
    {
        int size = RAM_BANK_SIZE * getRAMBankCount(memory->rom);
        loadFromFlash(memory->cartRam, 0, size);
    }
}

struct MiscSaveStateData
{
    struct CPUState cpu;
    char cpuPadding[128 - sizeof(struct CPUState)];
    struct AudioRenderState audioState;
};

int loadGameboyState(struct GameBoy* gameboy)
{
    int offset = 0;
    int sectionSize;
    sectionSize = RAM_BANK_SIZE * getRAMBankCount(gameboy->memory.rom);
    if (loadFromFlash(gameboy->memory.cartRam, offset, sectionSize) == -1)
    {
        return -1;
    }
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

    sectionSize = sizeof(struct GraphicsMemory);
    if (loadFromFlash(&gameboy->memory.vram, offset, sectionSize) == -1)
    {
        return -1;
    }
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

    sectionSize = MAX_RAM_SIZE;
    if (loadFromFlash(gameboy->memory.internalRam, offset, sectionSize) == -1)
    {
        return -1;
    }
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);
    
    sectionSize = sizeof(struct MiscMemory);
    if (loadFromFlash(&gameboy->memory.misc, offset, sectionSize) == -1)
    {
        return -1;
    }
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

    struct MiscSaveStateData miscData;
    sectionSize = sizeof(struct MiscSaveStateData);
    if (loadFromFlash(&miscData, offset, sectionSize) == -1)
    {
        return -1;
    }
    gameboy->cpu = miscData.cpu;
    gameboy->memory.audio.renderState = miscData.audioState;
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

    gameboy->memory.bankSwitch(&gameboy->memory, -1, 0);
    // TODO VRAM Bank/Internal GBC RAM Bank

    return 0;
}

int saveGameboyState(struct GameBoy* gameboy)
{    
    if (osFlashAllErase()) {
        return -1;
    }
    int offset = 0;
    int sectionSize = RAM_BANK_SIZE * getRAMBankCount(gameboy->memory.rom);
    saveToFlash(gameboy->memory.cartRam, offset, sectionSize);
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

    sectionSize = sizeof(struct GraphicsMemory);
    saveToFlash(&gameboy->memory.vram, offset, sectionSize);
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

    sectionSize = MAX_RAM_SIZE;
    saveToFlash(gameboy->memory.internalRam, offset, sectionSize);
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);
    
    sectionSize = sizeof(struct MiscMemory);
    saveToFlash(&gameboy->memory.misc, offset, sectionSize);
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

    struct MiscSaveStateData miscData;
    miscData.cpu = gameboy->cpu;
    miscData.audioState = gameboy->memory.audio.renderState;
    sectionSize = sizeof(struct MiscSaveStateData);
    saveToFlash(&miscData, offset, sectionSize);
    offset = ALIGN_FLASH_OFFSET(offset + sectionSize);

    return 0;
}

void initGameboy(struct GameBoy* gameboy, struct ROMLayout* rom)
{
    initializeCPU(&gameboy->cpu);
    initMemory(&gameboy->memory, rom);
    loadBIOS(gameboy->memory.rom, 0);
    loadRAM(&gameboy->memory);
    
    gameboy->cpu.a = 0x0;
    gameboy->cpu.f = 0x0;
    gameboy->cpu.b = 0x0;
    gameboy->cpu.c = 0x0;
    gameboy->cpu.d = 0x0;
    gameboy->cpu.e = 0x0;
    gameboy->cpu.h = 0x0;
    gameboy->cpu.l = 0x0;
    gameboy->cpu.pc = 0x0;
    gameboy->cpu.sp = 0x0;

    WRITE_REGISTER_DIRECT(&gameboy->memory, REG_LCDC, 0x00);
}

void requestInterrupt(struct GameBoy* gameboy, int interrupt)
{
    int interrupts;
    interrupts = READ_REGISTER_DIRECT(&gameboy->memory, REG_INT_REQUESTED);
    interrupts |= interrupts;
    WRITE_REGISTER_DIRECT(&gameboy->memory, REG_INT_REQUESTED, interrupt);

    if (
        (interrupt & READ_REGISTER_DIRECT(&gameboy->memory, REG_INT_ENABLED))
    )
    {
        struct CPUStoppingPoint stoppingPoint;
        stoppingPoint.cycleTime = gameboy->cpu.cyclesRun;
        stoppingPoint.stoppingPointType = CPUStoppingPointTypeInterrupt;
        addStoppingPoint(&gameboy->cpu, stoppingPoint);
        
        if (gameboy->cpu.stopReason == STOP_REASON_HALT || gameboy->cpu.stopReason == STOP_REASON_STOP)
        {
            gameboy->cpu.stopReason = STOP_REASON_NONE;
        }
    }
}

void emulateFrame(struct GameBoy* gameboy, void* targetMemory)
{
    struct GraphicsState graphicsState;
    int line;
    int catchupCycles;
    int screenWasEnabled;
    int ly;

    screenWasEnabled = READ_REGISTER_DIRECT(&gameboy->memory, REG_LCDC) & LCDC_LCD_E;

    if (gameboy->cpu.stopReason == STOP_REASON_STOP && (READ_REGISTER_DIRECT(&gameboy->memory, REG_KEY1) & REG_KEY1_SPEED_REQUEST))
    {
        // TODO change timing
        WRITE_REGISTER_DIRECT(&gameboy->memory, REG_KEY1, REG_KEY1_SPEED ^ READ_REGISTER_DIRECT(&gameboy->memory, REG_KEY1) & ~REG_KEY1_SPEED_REQUEST);
        gameboy->cpu.stopReason = STOP_REASON_NONE;
    }

    if (targetMemory && screenWasEnabled)
    {
        ly = READ_REGISTER_DIRECT(&gameboy->memory, REG_LY);

        if (ly != 0 || (READ_REGISTER_DIRECT(&gameboy->memory, REG_LCD_STAT) & REG_LCD_STAT_MODE) != 2)
        {
            // Run the CPU until the start of the next frame
            gameboy->cpu.runUntilNextFrame = 1;
            runCPU(
                &gameboy->cpu, 
                &gameboy->memory, 
                (2 + GB_SCREEN_LINES - ly) * CYCLES_PER_LINE
            );
            gameboy->cpu.runUntilNextFrame = 0;
        }

        initGraphicsState(&gameboy->memory, &graphicsState, 0);

        runCPU(&gameboy->cpu, &gameboy->memory, MODE_2_CYCLES);
        for (line = 0; line < GB_SCREEN_H; ++line)
        {
            graphicsState.row = line;
		    renderPixelRow(&gameboy->memory, &graphicsState, targetMemory);
            runCPU(&gameboy->cpu, &gameboy->memory, CYCLES_PER_LINE);

        }

        runCPU(&gameboy->cpu, &gameboy->memory, CYCLES_PER_LINE * V_BLANK_LINES - MODE_2_CYCLES);
    }
    else
    {
        runCPU(&gameboy->cpu, &gameboy->memory, CYCLES_PER_FRAME);
    }

    tickAudio(&gameboy->memory, gameboy->cpu.unscaledCyclesRun);
    adjustCPUTimer(&gameboy->cpu);

    if (gameboy->cpu.unscaledCyclesRun >= MAX_CYCLE_TIME)
    {
        gameboy->cpu.unscaledCyclesRun -= MAX_CYCLE_TIME;
        gameboy->memory.audio.renderState.cyclesEmulated -= MAX_CYCLE_TIME;
    }
}

void handleGameboyInput(struct GameBoy* gameboy, OSContPad* pad)
{
    int button;
    int joy;
    int nextJoy;

    button = 0xFF;

    if (pad->button & CONT_A)
        button &= ~GB_BUTTON_A;

    if (pad->button & CONT_B)
        button &= ~GB_BUTTON_B;

    if (pad->button & CONT_START)
        button &= ~GB_BUTTON_START;

    if (pad->button & Z_TRIG)
        button &= ~GB_BUTTON_SELECT;

    if ((pad->button & U_JPAD) || pad->stick_y > 0x40)
        button &= ~GB_BUTTON_UP;
    
    if ((pad->button & L_JPAD) || pad->stick_x < -0x40)
        button &= ~GB_BUTTON_LEFT;
    
    if ((pad->button & R_JPAD) || pad->stick_x > 0x40)
        button &= ~GB_BUTTON_RIGHT;
    
    if ((pad->button & D_JPAD) || pad->stick_y < -0x40)
        button &= ~GB_BUTTON_DOWN;

    WRITE_REGISTER_DIRECT(&gameboy->memory, _REG_JOYSTATE, button);

    joy = READ_REGISTER_DIRECT(&gameboy->memory, REG_JOYP);

    nextJoy = joy | 0x0F;

    if (!(joy & 0x10))
    {
        nextJoy &= button | 0xF0;
    }

    if (!(joy & 0x20))
    {
        nextJoy &= (button >> 4) | 0xF0;
    }

    if ((joy ^ nextJoy) & ~nextJoy & 0xF)
    {
        requestInterrupt(gameboy, GB_INTERRUPTS_INPUT);
    }
    
    WRITE_REGISTER_DIRECT(&gameboy->memory, REG_JOYP, nextJoy);
}

void unloadBIOS(struct Memory* memory)
{
    loadRomSegment(memory->rom->mainBank, memory->rom->romLocation, 0);
    int index;
    for (index = 0; index < BREAK_POINT_COUNT; ++index)
    {
        reapplyBreakpoint(&memory->breakpoints[index]);
    }
}