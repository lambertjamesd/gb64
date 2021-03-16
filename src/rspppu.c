
#include "rspppu.h"
#include "rspppu_includes.h"

#include <ultra64.h>

extern long long int ppuTextStart[];
extern long long int ppuTextEnd[];

extern long long int ppuDataStart[];
extern long long int ppuDataEnd[];

static char __attribute__((aligned(8))) outputBuffer[64];

extern OSMesgQueue     dmaMessageQ;
extern OSMesg          dmaMessageBuf;
extern OSPiHandle	   *handler;
extern OSIoMesg        dmaIOMessageBuf;

struct PPUTask
{
    u8* output;
    struct Memory* memorySource;
    struct GraphicsMemory* graphics;
    short flags;
    char lcdc;
    char ly;
    char scy;
    char scx;
    char wy;
    char wx;
    int padding;
};

static struct PPUTask __attribute__((aligned(8))) gPPUTask;
extern u8 __attribute__((aligned(8))) gScreenBuffer[];

void setupPPU()
{

}

void startPPUFrame(struct Memory* memory, int gbc)
{
    OSTask task;

    gPPUTask.output = (u8*)K0_TO_PHYS(gScreenBuffer);
    gPPUTask.memorySource = (struct Memory*)K0_TO_PHYS(memory);
    gPPUTask.graphics = (struct GraphicsMemory*)K0_TO_PHYS(&memory->vram);
    gPPUTask.flags = 0;
    gPPUTask.lcdc = READ_REGISTER_DIRECT(memory, REG_LCDC);
    gPPUTask.ly = READ_REGISTER_DIRECT(memory, REG_LY);
    gPPUTask.scy = READ_REGISTER_DIRECT(memory, REG_SCY);
    gPPUTask.scx = READ_REGISTER_DIRECT(memory, REG_SCX);
    gPPUTask.wy = READ_REGISTER_DIRECT(memory, REG_WY);
    gPPUTask.wx = READ_REGISTER_DIRECT(memory, REG_WX);

    if (gbc) {
        gPPUTask.flags |= PPU_TASK_FLAGS_COLOR;
    }

    task.t.type = M_GFXTASK;
    task.t.flags = OS_TASK_DP_WAIT;
    task.t.ucode_boot = (u64*)ppuTextStart;
    task.t.ucode_boot_size = (char*)ppuTextEnd - (char*)ppuTextStart;
    task.t.ucode = NULL;
    task.t.ucode_size = 0;
    task.t.ucode_data = (u64*)ppuDataStart;
    task.t.ucode_data_size = (char*)ppuDataEnd - (char*)ppuDataStart;
    task.t.dram_stack = 0;
    task.t.dram_stack_size = 0;
    task.t.output_buff = (u64*)outputBuffer;
    task.t.output_buff_size = 0;
    task.t.data_ptr = (u64*)&gPPUTask;
    task.t.data_size = sizeof(struct PPUTask);
    task.t.yield_data_ptr = 0;
    task.t.yield_data_size = 0;

    int i;
    for (i = 0; i < 32; ++i) {
        memory->vram.tilemap0[i] = i;
    }

    for (i = 0; i < 512; ++i) {
        memory->vramBytes[i] = i;
    }

    osWritebackDCache(memory->vram.tilemap0, 32);
    osWritebackDCache(memory->vramBytes, 512);
    osWritebackDCache(&gPPUTask, sizeof(struct PPUTask));

    osSpTaskStart(&task);

    int currentStatus;

    do
    {
        currentStatus = IO_READ(SP_STATUS_REG);
    } while (~currentStatus & SP_STATUS_HALT);

    currentStatus = 0;
}

void renderPPURow(struct Memory* memory)
{
    gPPUTask.lcdc = READ_REGISTER_DIRECT(memory, REG_LCDC);
    gPPUTask.ly = READ_REGISTER_DIRECT(memory, REG_LY);
    gPPUTask.scy = READ_REGISTER_DIRECT(memory, REG_SCY);
    gPPUTask.scx = READ_REGISTER_DIRECT(memory, REG_SCX);
    gPPUTask.wy = READ_REGISTER_DIRECT(memory, REG_WY);
    gPPUTask.wx = READ_REGISTER_DIRECT(memory, REG_WX);
    osWritebackDCache(&gPPUTask, sizeof(struct PPUTask));

    // set mode 3 bit
    IO_WRITE(SP_STATUS_REG, SP_SET_SIG0);
}