
#include "gameboy.h"
#include "graphics.h"

struct GameBoy gGameboy;

void initGameboy(struct GameBoy* gameboy, struct ROMLayout* rom)
{
    initializeCPU(&gameboy->cpu);
    initMemory(&gameboy->memory, rom);
    loadBIOS(gameboy->memory.rom, 0);

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
        if (gameboy->cpu.interrupts && !gameboy->cpu.nextInterrupt)
        {
            gameboy->cpu.nextInterrupt = interrupt;
            gameboy->cpu.nextInterruptTrigger = gameboy->cpu.cyclesRun + 1;
        }
        
        if (gameboy->cpu.stopReason == STOP_REASON_HALT || gameboy->cpu.stopReason == STOP_REASON_STOP)
        {
            gameboy->cpu.stopReason = STOP_REASON_NONE;
        }
    }
}

void emulateFrame(struct GameBoy* gameboy, void* targetMemory)
{
    int line;
    int catchupCycles;
    int screenWasEnabled;
    int ly;

    screenWasEnabled = READ_REGISTER_DIRECT(&gameboy->memory, REG_LCDC) & LCDC_LCD_E;

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
                (GB_SCREEN_LINES - ly) * CYCLES_PER_LINE
            );
            gameboy->cpu.runUntilNextFrame = 0;
        }

        runCPU(&gameboy->cpu, &gameboy->memory, MODE_2_CYCLES);
        for (line = 0; line < GB_SCREEN_H; ++line)
        {
		    renderPixelRow(&gameboy->memory, targetMemory, line, 0);
            runCPU(&gameboy->cpu, &gameboy->memory, CYCLES_PER_LINE);
        }

        runCPU(&gameboy->cpu, &gameboy->memory, CYCLES_PER_LINE * V_BLANK_LINES - MODE_2_CYCLES);
    }
    else
    {
        runCPU(&gameboy->cpu, &gameboy->memory, CYCLES_PER_FRAME);
    }
}

void handleInput(struct GameBoy* gameboy, OSContPad* pad)
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

    if (joy & 0x20)
    {
        nextJoy = (joy & 0xF0) | (button & 0xF);
    }
    else
    {
        nextJoy = (joy & 0xF0) | ((button >> 4) & 0xF);
    }

    if ((joy ^ nextJoy) & ~nextJoy & 0xF)
    {
        requestInterrupt(gameboy, GB_INTERRUPTS_INPUT);
    }
    
    WRITE_REGISTER_DIRECT(&gameboy->memory, REG_JOYP, nextJoy);
}