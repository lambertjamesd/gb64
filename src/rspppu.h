
#ifndef _RSP_PPU_H
#define _RSP_PPU_H

#include "memory_map.h"

void setupPPU();
void startPPUFrame(struct Memory* memory, int gdc);
void renderPPURow(struct Memory* memory);

#endif