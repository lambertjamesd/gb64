#include "./alias.s"
#include "./ostask.s"
#include "./registers.s"
#include "./pputask.s"
#include "./gameboy.s"

#include "../src/memory_map_offsets.h"
#include "../src/rspppu_includes.h"
    
.data
#include "data.s"
    .dmax 4032

.text 0x0
#include "main.s"
#include "dma.s"
#include "tilemap.s"
#include "sprite.s"
    .dmax 4096