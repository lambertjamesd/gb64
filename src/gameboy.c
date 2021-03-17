
#include "gameboy.h"
#include "graphics.h"
#include "debugger.h"
#include "debug_out.h"
#include "../render.h"
#include "../memory.h"
#include "upgrade.h"
#include "save.h"
#include "rspppu.h"

struct GameBoy gGameboy;

struct GameboySettings gDefaultSettings = {
    GB_SETTINGS_HEADER,
    GB_SETTINGS_CURRENT_VERSION,
    0,
    0,
    0,
    0,
    {
        InputButtonSetting_RD,
        InputButtonSetting_LD,
        InputButtonSetting_UD,
        InputButtonSetting_DD,
        
        InputButtonSetting_A,
        InputButtonSetting_B,
        InputButtonSetting_Z,
        InputButtonSetting_START,
        
        InputButtonSetting_DC,
        InputButtonSetting_UC,
        InputButtonSetting_LC,
        InputButtonSetting_RC,
    },
    {
        0,
        0,
        0,
    },
};

void initGameboy(struct GameBoy* gameboy, struct ROMLayout* rom)
{    
    initializeCPU(&gameboy->cpu);
    initMemory(&gameboy->memory, rom);
    gameboy->memory.misc.biosLoaded = 1;

    enum StoredInfoType saveType = getStoredInfoType(gameboy);

    loadSettings(gameboy, saveType);

    if (gameboy->settings.header != GB_SETTINGS_HEADER)
    {
        gameboy->settings = gDefaultSettings;
    }
    else
    {
        updateToLatestVersion(gameboy);
        loadRAM(&gameboy->memory, saveType);
    }


    if (gameboy->memory.rom->mainBank[GB_ROM_H_GBC_FLAG] == GB_ROM_GBC_ONLY || 
        gameboy->memory.rom->mainBank[GB_ROM_H_GBC_FLAG] == GB_ROM_GBC_SUPPORT && !(gameboy->settings.flags & GB_SETTINGS_FLAGS_DISABLE_GBC)
    ) {
        gameboy->cpu.gbc = 1;

        int i;

        for (i = 0; i < PALETTE_COUNT; ++i)
        {
            gGameboy.memory.vram.colorPalettes[i] = 0xFFFF;
        }
    }
    else
    {
        updatePaletteInfo(gameboy);
    }

    loadBIOS(gameboy->memory.rom, gameboy->cpu.gbc);
    
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
    gameboy->memory.misc.time = gameboy->settings.timer;
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

void emulateFrame(struct GameBoy* gameboy, bool renderScreen)
{
    struct GraphicsState graphicsState;
    int line;
    int catchupCycles;
    int screenWasEnabled;
    int ly;
    u64 accumulatedTime = 0;

    screenWasEnabled = READ_REGISTER_DIRECT(&gameboy->memory, REG_LCDC) & LCDC_LCD_E;

    if (renderScreen && screenWasEnabled)
    {
        // clear mode 3 bit
        IO_WRITE(SP_STATUS_REG, SP_CLR_SIG0);
        startPPUFrame(&gameboy->memory, gameboy->cpu.gbc);

        ly = READ_REGISTER_DIRECT(&gameboy->memory, REG_LY);

        if (ly != 0 || (READ_REGISTER_DIRECT(&gameboy->memory, REG_LCD_STAT) & REG_LCD_STAT_MODE) != 2)
        {
            // Run the CPU until the start of the next frame
            gameboy->cpu.runUntilNextFrame = 1;
            accumulatedTime += runCPU(
                &gameboy->cpu, 
                &gameboy->memory, 
                (2 + GB_SCREEN_LINES - ly) * CYCLES_PER_LINE,
                RUN_CPU_FLAGS_RENDER
            );
            gameboy->cpu.runUntilNextFrame = 0;
        }

        initGraphicsState(&gameboy->memory, &graphicsState, &gameboy->settings.graphics, gameboy->cpu.gbc);

        int cyclesToRun = CYCLES_TIL_LINE_RENDER;

        cyclesToRun -= runCPU(&gameboy->cpu, &gameboy->memory, cyclesToRun, RUN_CPU_FLAGS_RENDER);

        for (line = 0; line < GB_SCREEN_H; ++line)
        {
            if (line == 0) {
            }

            graphicsState.row = line;

            renderPPURow(&gameboy->memory);
		    renderPixelRow(&gameboy->memory, &graphicsState);
            cyclesToRun += CYCLES_PER_LINE;
            cyclesToRun -= runCPU(&gameboy->cpu, &gameboy->memory, cyclesToRun, RUN_CPU_FLAGS_RENDER);
        }

        finishScreen(&graphicsState);

        // intentionally run short to make sure we don't leak into the next frame
        cyclesToRun += CYCLES_PER_LINE * V_BLANK_LINES - CYCLES_TIL_LINE_RENDER - 2;
        cyclesToRun -= runCPU(&gameboy->cpu, &gameboy->memory, cyclesToRun, RUN_CPU_FLAGS_RENDER);

        accumulatedTime += CYCLES_PER_FRAME;
    }
    else
    {
        runCPU(&gameboy->cpu, &gameboy->memory, CYCLES_PER_FRAME, 0);
        accumulatedTime += CYCLES_PER_FRAME;
    }

    tickAudio(&gameboy->memory, gameboy->cpu.unscaledCyclesRun);
    adjustCPUTimer(&gameboy->cpu);

    if (gameboy->cpu.unscaledCyclesRun >= MAX_CYCLE_TIME)
    {
        gameboy->cpu.unscaledCyclesRun -= MAX_CYCLE_TIME;
        gameboy->memory.audio.cyclesEmulated -= MAX_CYCLE_TIME;
    }

    if (!(READ_REGISTER_DIRECT(&gameboy->memory, REG_RTC_DH) & REG_RTC_DH_HALT)) 
    {
        gameboy->memory.misc.time += accumulatedTime;
    }
}

void handleGameboyInput(struct GameBoy* gameboy, OSContPad* pad)
{
    int button;
    int joy;
    int nextJoy;

    button = 0xFF;

    if (pad->button & INPUT_BUTTON_TO_MASK(gameboy->settings.inputMapping.a))
        button &= ~GB_BUTTON_A;

    if (pad->button & INPUT_BUTTON_TO_MASK(gameboy->settings.inputMapping.b))
        button &= ~GB_BUTTON_B;

    if (pad->button & INPUT_BUTTON_TO_MASK(gameboy->settings.inputMapping.start))
        button &= ~GB_BUTTON_START;

    if (pad->button & INPUT_BUTTON_TO_MASK(gameboy->settings.inputMapping.select))
        button &= ~GB_BUTTON_SELECT;

    if ((pad->button & INPUT_BUTTON_TO_MASK(gameboy->settings.inputMapping.up)) || pad->stick_y > 0x40)
        button &= ~GB_BUTTON_UP;
    
    if ((pad->button & INPUT_BUTTON_TO_MASK(gameboy->settings.inputMapping.left)) || pad->stick_x < -0x40)
        button &= ~GB_BUTTON_LEFT;
    
    if ((pad->button & INPUT_BUTTON_TO_MASK(gameboy->settings.inputMapping.right)) || pad->stick_x > 0x40)
        button &= ~GB_BUTTON_RIGHT;
    
    if ((pad->button & INPUT_BUTTON_TO_MASK(gameboy->settings.inputMapping.down)) || pad->stick_y < -0x40)
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
    memory->misc.biosLoaded = 0;
    int index;
    for (index = 0; index < BREAK_POINT_COUNT; ++index)
    {
        reapplyBreakpoint(&memory->breakpoints[index]);
    }
}

enum InputButtonSetting getButtonMapping(struct InputMapping* inputMapping, enum InputButtonIndex buttonIndex)
{
    return ((u8*)inputMapping)[buttonIndex];
}

void setButtonMapping(struct InputMapping* inputMapping, enum InputButtonIndex buttonIndex, enum InputButtonSetting setting)
{
    u8* asInputArray = (u8*)inputMapping;
    enum InputButtonSetting existingSetting = asInputArray[buttonIndex];
    int i;
    for (i = 0; i < InputButtonIndexCount; ++i)
    {
        if (asInputArray[i] == setting)
        {
            asInputArray[i] = existingSetting;
        }
    }
    asInputArray[buttonIndex] = setting;
}

u16 gDMAPalettes[][4] = {
    {
        COL24TO16(0xE0F8D0), 
        COL24TO16(0x88C070), 
        COL24TO16(0x346856), 
        COL24TO16(0x081820),
    },
    {
        COL24TO16(0xf8f8f8), 
        COL24TO16(0xb8b8b8), 
        COL24TO16(0x707070), 
        COL24TO16(0x000000),
    },
    {
        COL24TO16(0xFCE4A8), 
        COL24TO16(0x71969F), 
        COL24TO16(0xD71A21), 
        COL24TO16(0x00324D),
    },
    // SGB palettes
    {
        COL24TO16(0xf8e8c8), 
        COL24TO16(0xd89048), 
        COL24TO16(0xa82820), 
        COL24TO16(0x301850),
    },
    {
        COL24TO16(0xd8d8c0), 
        COL24TO16(0xc8b070), 
        COL24TO16(0xb05010), 
        COL24TO16(0x000000),
    },
    {
        COL24TO16(0xf8c0f8), 
        COL24TO16(0xe89850), 
        COL24TO16(0x983860), 
        COL24TO16(0x383898),
    },
    {
        COL24TO16(0xf8f8a8), 
        COL24TO16(0xc08048), 
        COL24TO16(0xf80000), 
        COL24TO16(0x501800),
    },
    {
        COL24TO16(0xf8d8b0), 
        COL24TO16(0x78c078), 
        COL24TO16(0x688840), 
        COL24TO16(0x583820),
    },
    {
        COL24TO16(0xd8e8f8), 
        COL24TO16(0xe08850), 
        COL24TO16(0xa80000), 
        COL24TO16(0x004010),
    },
    {
        COL24TO16(0x000050), 
        COL24TO16(0x00a0e8), 
        COL24TO16(0x787800), 
        COL24TO16(0xf8f858),
    },
    {
        COL24TO16(0xf8e8e0), 
        COL24TO16(0xf8b888), 
        COL24TO16(0x804000), 
        COL24TO16(0x301800),
    },
    
    {
        COL24TO16(0xf0c8a0), 
        COL24TO16(0xc08848), 
        COL24TO16(0x287800), 
        COL24TO16(0x000000),
    },
    {
        COL24TO16(0xf8f8f8), 
        COL24TO16(0xf8e850), 
        COL24TO16(0xf83000), 
        COL24TO16(0x500058),
    },
    {
        COL24TO16(0xf8c0f8), 
        COL24TO16(0xe88888), 
        COL24TO16(0x7830e8), 
        COL24TO16(0x282898),
    },
    {
        COL24TO16(0xf8f8a0), 
        COL24TO16(0x00f800), 
        COL24TO16(0xf83000), 
        COL24TO16(0x000050),
    },
    {
        COL24TO16(0xf8c880), 
        COL24TO16(0x90b0e0), 
        COL24TO16(0x281060), 
        COL24TO16(0x100810),
    },
    {
        COL24TO16(0xd0f8f8), 
        COL24TO16(0xf89050), 
        COL24TO16(0xa00000), 
        COL24TO16(0x180000),
    },
    {
        COL24TO16(0x68b838), 
        COL24TO16(0xe05040), 
        COL24TO16(0xe0b880), 
        COL24TO16(0x001800),
    },
    
    {
        COL24TO16(0xf8d098), 
        COL24TO16(0x70c0c0), 
        COL24TO16(0xf86028), 
        COL24TO16(0x304860),
    },
    {
        COL24TO16(0xd8d8c0), 
        COL24TO16(0xe08020), 
        COL24TO16(0x005000), 
        COL24TO16(0x001010),
    },
    {
        COL24TO16(0xe0a8c8), 
        COL24TO16(0xf8f878), 
        COL24TO16(0x00b8f8), 
        COL24TO16(0x202058),
    },
    {
        COL24TO16(0xf0f8b8), 
        COL24TO16(0xe0a878), 
        COL24TO16(0x08c800), 
        COL24TO16(0x000000),
    },
    {
        COL24TO16(0xf8f8c0), 
        COL24TO16(0xe0b068), 
        COL24TO16(0xb07820), 
        COL24TO16(0x504870),
    },
    {
        COL24TO16(0x7878c8), 
        COL24TO16(0xf868f8), 
        COL24TO16(0xf8d000), 
        COL24TO16(0x404040),
    },
    {
        COL24TO16(0xf8f8f8), 
        COL24TO16(0x60d850), 
        COL24TO16(0xc83038), 
        COL24TO16(0x380000),
    },
    
    {
        COL24TO16(0xf0a868), 
        COL24TO16(0x78a8f8), 
        COL24TO16(0xd000d0), 
        COL24TO16(0x000078),
    },
    {
        COL24TO16(0xf0e8f0), 
        COL24TO16(0xe8a060), 
        COL24TO16(0x407838), 
        COL24TO16(0x180808),
    },
    {
        COL24TO16(0xf8e0e0), 
        COL24TO16(0xd8a0d0), 
        COL24TO16(0x98a0e0), 
        COL24TO16(0x080000),
    },
    {
        COL24TO16(0xf8f8b8), 
        COL24TO16(0x90c8c8), 
        COL24TO16(0x486878), 
        COL24TO16(0x082048),
    },
    {
        COL24TO16(0xf8d8a8), 
        COL24TO16(0xe0a878), 
        COL24TO16(0x785888), 
        COL24TO16(0x002030),
    },
    {
        COL24TO16(0xb8d0d0), 
        COL24TO16(0xd880d8), 
        COL24TO16(0x8000a0), 
        COL24TO16(0x380000),
    },
    {
        COL24TO16(0xb0e018), 
        COL24TO16(0xb82058), 
        COL24TO16(0x281000), 
        COL24TO16(0x008060),
    },
    {
        COL24TO16(0xf8f8c8), 
        COL24TO16(0xb8c058), 
        COL24TO16(0x808840), 
        COL24TO16(0x405028),
    },
};

int getPaletteCount()
{
    return sizeof(gDMAPalettes) / sizeof(*gDMAPalettes);
}

u16* getPalette(int index)
{
    if (index >= 0 && index < getPaletteCount())
    {
        return gDMAPalettes[index];
    }
    else
    {
        return gDMAPalettes[0];
    }
}

void updatePaletteInfo(struct GameBoy* gameboy)
{
    u16* src = getPalette(gameboy->settings.bgpIndex);

    (&gBGPColors)[0] = src[0]; (&gBGPColors)[1] = src[1];
    (&gBGPColors)[2] = src[2]; (&gBGPColors)[3] = src[3];

    src = getPalette(gameboy->settings.obp0Index);
    (&gOBP0Colors)[0] = src[0]; (&gOBP0Colors)[1] = src[1];
    (&gOBP0Colors)[2] = src[2]; (&gOBP0Colors)[3] = src[3];
    
    src = getPalette(gameboy->settings.obp1Index);
    (&gOBP1Colors)[0] = src[0]; (&gOBP1Colors)[1] = src[1];
    (&gOBP1Colors)[2] = src[2]; (&gOBP1Colors)[3] = src[3];

    u8 bgp = READ_REGISTER_DIRECT(&gameboy->memory, REG_BGP);
    gameboy->memory.vram.colorPalettes[0] = (&gBGPColors)[bgp >> 0 & 0x3];
    gameboy->memory.vram.colorPalettes[1] = (&gBGPColors)[bgp >> 2 & 0x3];
    gameboy->memory.vram.colorPalettes[2] = (&gBGPColors)[bgp >> 4 & 0x3];
    gameboy->memory.vram.colorPalettes[3] = (&gBGPColors)[bgp >> 6 & 0x3];
    
    u8 obp0 = READ_REGISTER_DIRECT(&gameboy->memory, REG_OBP0);
    gameboy->memory.vram.colorPalettes[32] = (&gOBP0Colors)[obp0 >> 0 & 0x3];
    gameboy->memory.vram.colorPalettes[33] = (&gOBP0Colors)[obp0 >> 2 & 0x3];
    gameboy->memory.vram.colorPalettes[34] = (&gOBP0Colors)[obp0 >> 4 & 0x3];
    gameboy->memory.vram.colorPalettes[35] = (&gOBP0Colors)[obp0 >> 6 & 0x3];
    
    u8 obp1 = READ_REGISTER_DIRECT(&gameboy->memory, REG_OBP1);
    gameboy->memory.vram.colorPalettes[36] = (&gOBP1Colors)[obp1 >> 0 & 0x3];
    gameboy->memory.vram.colorPalettes[37] = (&gOBP1Colors)[obp1 >> 2 & 0x3];
    gameboy->memory.vram.colorPalettes[38] = (&gOBP1Colors)[obp1 >> 4 & 0x3];
    gameboy->memory.vram.colorPalettes[39] = (&gOBP1Colors)[obp1 >> 6 & 0x3];
}