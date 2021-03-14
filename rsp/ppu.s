
#include "./ostask.s"
#include "./registers.s"
#include "./pputask.s"
#include "./gameboy.s"

.name zero, $0
.name return, $31

.name p0, $4
.name p1, $5
.name p2, $6
.name p3, $7

.name tmp, $1
    
.data

.align 8
ppuTask:
    .space PPUTask_sizeof

.align 8
scanline:
    .space GB_SCREEN_WD

.align 8
tilemap:
    .space GB_TILEMAP_W
tilemapAttrs:
    .space GB_TILEMAP_W
tilemapTileCache:
    .space GB_TILE_SIZE * GB_MAX_VISIBLE_TILES
tilemapTileCacheInfo:
    .space 2 * GB_MAX_VISIBLE_TILES

.align 8
spriteTileCache:
    .space GB_TILE_SIZE * GB_MAX_VISIBLE_SPRITES
spriteTileCacheInfo:
    .space 2 * GB_MAX_VISIBLE_SPRITES

    .dmax 4032

.text 0x0

ppuMain:
    ori p0, zero, 0
    lw p1, osTask_ucode_data($0)
    lw p2, osTask_ucode_data_size($0)
    addi p2, p2, -1
    jal DMAproc
    ori p3, zero, 0

    ori p0, zero, ppuTask
    lw p1, osTask_data_ptr($0)
    lw p2, osTask_data_ptr_size($0)
    addi p2, p2, -1
    jal DMAproc
    ori p3, zero, 0

    jal DMAWait
    # note delay slot

.name index, $8
.name output, $9
.name workingMemory, $10
.name pixelIndex, $11
.name step, $12
    ori index, zero, GB_SCREEN_WD
    ori workingMemory, zero, scanline
    lui pixelIndex, 0x0001
    ori pixelIndex, pixelIndex, 0x0203
    lui step, 0x0404
    ori step, step, 0x0404

xFillLoop:
    sw pixelIndex, 0(workingMemory)
    addi index, index, -4
    addi workingMemory, workingMemory, 4
    bne index, zero, xFillLoop
    add pixelIndex, pixelIndex, step

    ori index, zero, GB_SCREEN_HT
    ori p0, zero, scanline
    lw p1, (ppuTask + PPUTask_output)($0)
    ori p2, zero, GB_SCREEN_WD
    ori p3, zero, 1
yFillLoop:
    jal DMAproc
    addi index, index, -1
    bne index, zero, yFillLoop
    addi p1, p1, GB_SCREEN_WD

    jal DMAWait
    nop

.unname index
.unname output
.unname workingMemory
.unname pixelIndex
.unname step

    break
    .dmax 4096

###############################################
# Procedure to do DMA reads/writes.
# Registers:
#   p0 mem_addr
#   p1 dram_addr
#   p2 dma_len
#   p3 iswrite?
DMAproc: # request DMA access: (get semaphore)
    mfc0 tmp, SP_RESERVED
    bne tmp, zero, DMAproc
    # note delay slot
DMAFull: # wait for not FULL:
    mfc0 tmp, DMA_FULL
    bne tmp, zero, DMAFull
    nop
    # set DMA registers:
    mtc0 p0, DMA_CACHE
    # handle writes:
    bgtz p3, DMAWrite
    mtc0 p1, DMA_DRAM
    j DMADone
    mtc0 p2, DMA_READ_LENGTH
DMAWrite:
    mtc0 p2, DMA_WRITE_LENGTH
DMADone:
    jr return
    # clear semaphore, delay slot
    mtc0 zero, SP_RESERVED

###############################################
# Procedure to wait for DMA to finish
DMAWait:
    mfc0 tmp, SP_RESERVED
    bne tmp, zero, DMAWait
WaitSpin:
    mfc0 tmp, DMA_BUSY
    bne tmp, zero, WaitSpin
    nop
    jr $ra
    mtc0 zero, SP_RESERVED