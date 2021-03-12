
#include "rspppu.h"

#include <ultra64.h>

extern char ppuTextStart[];
extern char ppuTextEnd[];

extern char ppuDataStart[];
extern char ppuDataEnd[];

static char __attribute__((aligned(8))) outputBuffer[64];

extern OSMesgQueue     dmaMessageQ;
extern OSMesg          dmaMessageBuf;
extern OSPiHandle	   *handler;
extern OSIoMesg        dmaIOMessageBuf;

struct PPUTask
{
    u8* output;
    struct Memory* memorySource;
};

static struct PPUTask __attribute__((aligned(8))) gPPUTask;
extern u8 __attribute__((aligned(8))) gScreenBuffer[];

void setupPPU()
{

}

void startPPUFrame(struct Memory* memory)
{
    OSTask task;

    gPPUTask.output = (u8*)K0_TO_PHYS(gScreenBuffer);
    gPPUTask.memorySource = (struct Memory*)K0_TO_PHYS(memory);

    task.t.type = M_GFXTASK;
    task.t.flags = OS_TASK_DP_WAIT;
    task.t.ucode_boot = (u64*)ppuTextStart;
    task.t.ucode_boot_size = ppuTextEnd - ppuTextStart;
    task.t.ucode = NULL;
    task.t.ucode_size = 0;
    task.t.ucode_data = (u64*)ppuDataStart;
    task.t.ucode_data_size = ppuDataEnd - ppuDataStart;
    task.t.dram_stack = 0;
    task.t.dram_stack_size = 0;
    task.t.output_buff = (u64*)outputBuffer;
    task.t.output_buff_size = 0;
    task.t.data_ptr = (u64*)&gPPUTask;
    task.t.data_size = sizeof(struct PPUTask);
    task.t.yield_data_ptr = 0;
    task.t.yield_data_size = 0;

    osSpTaskStart(&task);

    int currentStatus;

    do
    {
        currentStatus = IO_READ(SP_STATUS_REG);
    } while (~currentStatus & SP_STATUS_HALT);

    currentStatus = 0;
}