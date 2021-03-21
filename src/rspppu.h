
#ifndef _RSP_PPU_H
#define _RSP_PPU_H

#include "memory_map.h"
#include "graphics.h"

void setupPPU();
void startPPUFrame(struct Memory* memory, int gdc);
void renderPPURow(struct Memory* memory, struct GraphicsState* state);

#endif