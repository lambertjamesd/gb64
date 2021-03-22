
#ifndef _RSP_PPU_H
#define _RSP_PPU_H

#include "memory_map.h"
#include "graphics.h"

struct PPUPerformance {
    int mode2StallCount;
    int mode3StallCount;
};

extern struct PPUPerformance gPPUPerformance;

void setupPPU();
void startPPUFrame(struct Memory* memory, int gdc);
void renderPPURow(struct Memory* memory, struct GraphicsState* state);

#endif