#include "./alias.s"
#include "./ostask.s"
#include "./registers.s"
#include "./pputask.s"
#include "./gameboy.s"

#include "../src/memory_map_offsets.h"
#include "../src/rspppu_includes.h"

.name zero, $0
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
# entry point
ppuMain: 
    # initialize stack pointer
    ori $sp, zero, stackEnd + STACK_SIZE

    # load ucode_data
    ori a0, zero, 0
    lw a1, osTask_ucode_data($0)
    lw a2, osTask_ucode_data_size($0)
    addi a2, a2, -1
    jal DMAproc
    ori a3, zero, 0

    # clear the MODE_3_FLAG bit
    li($at, MODE_3_FLAG_CLR)
    mtc0 $at, SP_STATUS

renderLine:
    # busy loop to wait for cpu to signal mode 3
    mfc0 $at, SP_STATUS
    andi $at, $at, MODE_3_FLAG
    beq $at, zero, renderLine

    # load PPUTask
    ori a0, zero, ppuTask
    lw a1, osTask_data_ptr($0)
    li(a2, PPUTask_sizeof-1)
    jal DMAproc
    ori a3, zero, 0

    # wait for PPUTask to finish loading
    jal DMAWait
    # note delay slot
    nop
    
    # get the tilemap values and attibutes
    jal precacheLine
    nop

    # reset the tile cache
    li($at, tilemapTileCache)
    sh $at, currentTile(zero)

    # get the tiles needed to render the current row
    li(a0, tilemap)
    jal precacheTiles
    li(a1, 20)

    # clear the MODE_3_FLAG bit
    ori $at, zero, MODE_3_FLAG_CLR
    mtc0 $at, SP_STATUS

    lbu $at, (ppuTask + PPUTask_ly)(zero)
    li(a0, 1)
    sb a0, scanline($at)

    jal writeScanline
    nop

    # reset the scanline buffer
    ori a0, zero, scanline
    lw a1, osTask_ucode_data($0)
    addi a1, a1, scanline
    ori a2, GB_SCREEN_WD-1
    jal DMAproc
    ori a3, zero, 0

    # check if this is the last line
    lbu $at, (ppuTask + PPUTask_ly)(zero)
    addi $at, $at, -(GB_SCREEN_HT - 1)

    # render the next line if there is more data to render
    bne $at, zero, renderLine
    nop

    break
    .dmax 4096

###############################################
# Main loop algorithm
# 1 setup initial state
# 2 wait for MODE 3 on the CPU to access VRAM
# 3 DMA Needed memory to DEMEM to render a single line
# 4 signal to CPU that VRAM has been read so it can 
#   continue to MODE 0
# 5 Render single line
# 6 If done with screen, break
# 7 Setup state for another scanline
# 8 goto step 2

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
    # check if the tile id matches
    lhu $at, tilemapTileCacheInfo(v1)
    beq $at, a0, nextTile # bail out early 
    addi $at, v0, GB_TILE_SIZE # increment tile pointer
    sh $at, currentTile(zero) # save tile pointer

    sh a0, tilemapTileCacheInfo(v1) # save new id

    lw a1, (ppuTask + PPUTask_graphicsSource)(zero)
    add a1, a1, a0 # load source of tile from main ram
    ori a0, v0, 0 # set the target in DMEM
    ori a2, zero, GB_TILE_SIZE - 1 # size of dma copy
    j DMAproc # copy tile into result
    ori a3, zero, 0 # mark as read

nextTile:
    jr $ra
    sh $at, currentTile(zero) # save tile pointer

###############################################
# Loads the tilemap row into memory

precacheLine:
    addi $sp, $sp, -4
    sw return, 0($sp)

    # get the current line
    lbu a1, (ppuTask + PPUTask_ly)(zero)
    # get the current screen offset
    lbu $at, (ppuTask + PPUTask_scy)(zero)
    sub a1, a1, $at # relative offset
    andi a1, a1, 0xF8 # wrap to 256 pixels and mask to tile
    srl a1, a1, 2 # get the tile offset in tilemap memory
    lbu $at, (ppuTask + PPUTask_lcdc)(zero)
    andi $at, $at, LCDC_BG_TILE_MAP
    # convert to a tilemap offset
    srl $at, $at, 7
    # store the offset into vram memory
    add a1, $at, a1
    lw $at, (ppuTask + PPUTask_graphicsSource)(zero)
    addi $at, $at, GraphicsMemory_tilemap0
    add a1, $at, a1 # calculate final ram address for tilemap row
    ori a0, zero, tilemap # dma target
    # intentially fall through

###############################################
# Loads the tilemap row into memory using a ram pointer
# a0 - target pointer into DMEM
# a1 - source pointer from ram
precacheLineFromPointer:
    ori a2, zero, GB_TILEMAP_W - 1 # dma length
    jal DMAproc # read tilemap row
    ori a3, zero, 0 # is read

    # check for gameboy color
    lhu $at, (ppuTask + PPUTask_flags)(zero)
    andi $at, $at, PPU_TASK_FLAGS_COLOR
    beq $at, zero, skipTilemapInfo
    # note delay slot
    addi a1, a1, GraphicsMemory_tilemapOffset # offset source from ram

    jal DMAproc
    ori a0, zero, (tilemapAttrs-tilemap) # offset target into DMEM

skipTilemapInfo:

    lw return, 0($sp)
    j DMAWait
    addi $sp, $sp, 4

###############################################
# Loads the tilemap row into memory
#
# a0 - address of tile source
# a1 - offset into tile source
# a2 - number of sprites to load

precacheTiles:
    addi $sp, $sp, -16
    sw return, 0($sp)
    sw s0, 4($sp)
    sw s1, 8($sp)
    sw s2, 12($sp)

    ori s0, a0, 0
    ori s1, a1, 0
    ori s2, a2, 0

precacheNextTile:
    beq s2, zero, precacheTilesFinish
    add a1, s0, s1 # get the tile address
    lbu a0, 0(a1) # load the tile index
    sll a0, a0, 4 # convert the tile index to a relative tile pointer

    # 
    # calculate which tile range to use
    #
    lbu $at, (ppuTask + PPUTask_lcdc)(zero)
    # determine which tile offset to use
    andi $at, $at, LCDC_BG_TILE_DATA 
    sll $at, $at, 3 # if set, LCDC_BG_TILE_DATA this values becomes 0x80
    # a when LCDC_BG_TILE_DATA is set
    # it should select the lower range
    xori $at, $at, 0x80 
    # selectes betweeen the tile range 8000-8FFF and 8800-97ff
    add a0, a0, $at
    andi a0, a0, 0xFF
    add a0, a0, $at

    #
    # calculate which tile bank to use
    #
    lbu $at, (tilemap - tilemapAttrs)(a1)
    andi $at, $at, TILE_ATTR_VRAM_BANK
    sll $at, $at, 10 # converts 0x08 flag to 0x2000 offset
    add a0, a0, $at

    jal requestTile
    addi s1, s1, 1
    andi s1, s1, 0x1f # wrap to 32 tiles
    j precacheNextTile
    addi s2, s2, -1

precacheTilesFinish:
    lw return, 0($sp)
    lw s0, 4($sp)
    lw s1, 8($sp)
    lw s2, 12($sp)

    jr return
    addi $sp, $sp, 16

###############################################
# Write scanline
#

writeScanline:
    # current the current y position
    lbu a0, (ppuTask + PPUTask_ly)(zero)
    # need to calculte ly * 160, since there is no
    # multiply instruction ly * 128 + ly * 32 is done
    # as it can be done with bit shifts and adds
    sll a1, a0, 7 
    sll a0, a0, 5
    add a1, a1, a0

    # get the output buffer
    lw $at, (ppuTask + PPUTask_output)(zero)
    # offset it by ly * 160
    add a1, a1, $at
    # load the source DMEM
    ori a0, zero, scanline
    ori a2, zero, GB_SCREEN_WD-1 # dma length
    j DMAproc # have DMAProc jump back to $ra
    ori a3, zero, 1 # is write