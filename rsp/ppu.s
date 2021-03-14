#include "./alias.s"
#include "./ostask.s"
#include "./registers.s"
#include "./pputask.s"
#include "./gameboy.s"

#include "../src/memory_map_offsets.h"
#include "../src/rspppu_includes.h"

.name zero, $0
.name sp, $30
.name return, $31

.name v0, $2
.name v1, $3

.name a0, $4
.name a1, $5
.name a2, $6
.name a3, $7

.name s0, $16
.name s1, $17
.name s2, $18
.name s3, $19
.name s4, $20
.name s5, $21
.name s6, $22
.name s7, $23
    
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

.align 8
window:
    .space GB_TILEMAP_W
windowAttrs:
    .space GB_TILEMAP_W

.align 8
tilemapTileCache:
    .space GB_TILE_SIZE * (GB_MAX_VISIBLE_TILES + GB_MAX_VISIBLE_SPRITES)
tilemapTileCacheInfo:
    .half -1
    .half -1
    .half -1
    .half -1
    .half -1

    .half -1
    .half -1
    .half -1
    .half -1
    .half -1

    .half -1
    .half -1
    .half -1
    .half -1
    .half -1

    .half -1
    .half -1
    .half -1
    .half -1
    .half -1

    .half -1
    .half -1
    .half -1
    .half -1
    .half -1

    .half -1
    .half -1
    .half -1
    .half -1
    .half -1

    .half -1

currentTile:
    .half tilemapTileCache

#define STACK_SIZE  64
stackEnd:
    .space STACK_SIZE

    .dmax 4032

.text 0x0
ppuMain: # entry point
    ori sp, zero, stackEnd + STACK_SIZE

    ori a0, zero, 0
    lw a1, osTask_ucode_data($0)
    lw a2, osTask_ucode_data_size($0)
    addi a2, a2, -1
    jal DMAproc
    ori a3, zero, 0

    ori a0, zero, ppuTask
    lw a1, osTask_data_ptr($0)
    li(a2, PPUTask_sizeof-1)
    jal DMAproc
    ori a3, zero, 0

    jal DMAWait
    # note delay slot
    nop
    
    jal precacheLine
    nop

    break
    .dmax 4096

###############################################
# Procedure to do DMA reads/writes.
# Registers:
#   a0 mem_addr
#   a1 dram_addr
#   a2 dma_len
#   a3 iswrite?
DMAproc: # request DMA access: (get semaphore)
    mfc0 $at, SP_RESERVED
    bne $at, zero, DMAproc
    # note delay slot
DMAFull: # wait for not FULL:
    mfc0 $at, DMA_FULL
    bne $at, zero, DMAFull
    nop
    # set DMA registers:
    mtc0 a0, DMA_CACHE
    # handle writes:
    bgtz a3, DMAWrite
    mtc0 a1, DMA_DRAM
    j DMADone
    mtc0 a2, DMA_READ_LENGTH
DMAWrite:
    mtc0 a2, DMA_WRITE_LENGTH
DMADone:
    jr return
    # clear semaphore, delay slot
    mtc0 zero, SP_RESERVED

###############################################
# Procedure to wait for DMA to finish
DMAWait:
    mfc0 $at, SP_RESERVED
    bne $at, zero, DMAWait
WaitSpin:
    mfc0 $at, DMA_BUSY
    bne $at, zero, WaitSpin
    nop
    jr $ra
    mtc0 zero, SP_RESERVED


###############################################
# Procedure to ensure the requested tile is
# loaded into DRAM
# Registers:
#   a0 the offset of the tile into struct GraphicsMemory
#
#   v0 pointer to the tile in DRAM
requestTile:
    # load the result, the pointer to the
    # current tile
    lhu v0, currentTile(zero)
    # retrieve the current tile ID
    addi v1, v0, -tilemapTileCache
    srl v1, v1, 3
    addi v1, v1, tilemapTileCacheInfo
    # check if the tile id matches
    lhu $at, GB_TILE_INFO_ID(v1)
    beq $at, a0, nextTile # bail out early 
    addi $at, a0, GB_TILE_SIZE # increment tile pointer
    sh $at, GB_TILE_INFO_ID(v1) # save tile pointer

    lw a1, (ppuTask + PPUTask_graphicsSource)(zero)
    add a1, a1, a0 # load source of tile from main ram
    ori a0, v0, 0 # set the target in DMEM
    ori a2, zero, GB_TILE_SIZE # size of dma copy
    j DMAproc # copy tile into result
    ori a3, zero, 0 # mark as read

nextTile:
    jr $ra
    sh $at, GB_TILE_INFO_ID(v1) # save tile pointer

precacheLine:
    addi sp, sp, -8
    sw return, 0(sp)

    # get the current line
    lbu a1, (ppuTask + PPUTask_ly)(zero)
    # get the current screen offset
    lbu $at, (ppuTask + PPUTask_scy)(zero)
    sub a1, a1, $at # relative offset
    andi a1, a1, 0xF8 # wrap to 256 pixels and mask to tile
    srl a1, a1, 2 # get the tile offset in tilemap memory
    lbu s0, (ppuTask + PPUTask_lcdc)(zero)
    andi $at, s0, REG_LCDC_GDB_TILE_MAP
    # convert to a tilemap offset
    srl $at, $at, 7
    # store the offset into vram memory
    add a1, $at, a1
    lbu $at, (ppuTask + PPUTask_graphicsSource)(zero)
    addi $at, $at, GraphicsMemory_tilemap0
    add a1, $at, a1 # calculate final ram address for tilemap row
    ori a0, zero, tilemap # dma target
    ori a2, zero, GB_TILEMAP_W # dma length
    jal DMAproc # read tilemap row
    ori a3, zero, 0 # is read

    # check for gameboy color
    lhu $at, (ppuTask + PPUTask_flags)(zero)
    andi $at, $at, PPU_TASK_FLAGS_COLOR
    beq $at, zero, skipTilemapInfo
    # note delay slot
    addi a1, a1, GraphicsMemory_tilemapOffset

    jal DMAproc
    ori a0, zero, tilemapAttrs

skipTilemapInfo:

    jr return
    addi sp, sp, 8
