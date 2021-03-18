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

.name t0, $8
.name t1, $9
.name t2, $10
.name t3, $11
.name t4, $12
.name t5, $13
.name t6, $14
.name t7, $15

.name s0, $16
.name s1, $17
.name s2, $18
.name s3, $19
.name s4, $20
.name s5, $21
.name s6, $22
.name s7, $23

.name t8, $24
.name t9, $25

.name k0, $26
.name k1, $27

.name gp, $28
    
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
    .half -1

currentTile:
    .half tilemapTileCache
currentWindowY:
    .half 0

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
    li(a2, LCDC_BG_TILE_MAP)

    # get the current line
    lbu a1, (ppuTask + PPUTask_ly)(zero)
    # get the current screen offset
    lbu $at, (ppuTask + PPUTask_scy)(zero)
    add a1, a1, $at # relative offset
    andi a1, a1, 0xFF # wrap to 256 pixels and mask to tile

    jal precacheLine
    ori a0, zero, tilemap # dma target

    # reset the tile cache
    li($at, tilemapTileCache)
    sh $at, currentTile(zero)

    # window x position
    li(s0, GB_SCREEN_WD)

    # check if the window is enabled
    lbu $at, (ppuTask + PPUTask_lcdc)(zero)
    andi $at, $at, LCDC_WIN_E
    beq $at, zero, precacheTilemap

    # check if the window is below the current row
    lbu t0, (ppuTask + PPUTask_ly)(zero)
    lbu t1, (ppuTask + PPUTask_wy)(zero)
    sub $at, t1, t0
    bgtz $at, precacheTilemap

    # check if window x is less than the screen width
    lbu t0, (ppuTask + PPUTask_wx)(zero)
    addi t0, t0, -WINDOW_X_OFFSET
    addi $at, t0, -GB_SCREEN_WD
    bgez $at, precacheTilemap

    lhu s1, currentWindowY(zero) # load current window y (note delay slot)

    # TODO handle negative window position
    ori s0, t0, 0 # set new window position

    addi $at, s1, 1 # increment and save current window y
    sh $at, currentWindowY(zero)

    li(a2, LCDC_WIN_TILE_MAP) # flag for selecting background source
    ori a1, s1, 0 # y line of row
    jal precacheLine
    ori a0, zero, window # dma target

precacheTilemap:
    #check if tilemap is visible
    blez s0, precacheWindow 

    lbu $at, (ppuTask + PPUTask_scx)(zero)
    srl a1, $at, 3
    add a2, $at, s0 # end pixel row at window position
    addi a2, a2, -1
    srl a2, a2, 3
    # calculate number of tiles
    sub a2, a2, a1 
    addi a2, a2, 1

    # get the tiles needed to render the current row
    jal precacheTiles
    li(a0, tilemap)

    # check if the window is visible
    li(a2, GB_SCREEN_WD)
    beq s0, a2, beginDrawingRow

precacheWindow:
    # calculate the number of tiles to render
    li(a2, GB_SCREEN_WD-1)
    sub a2, a2, s0 
    srl a2, a2, 3
    addi a2, a2, 1
    # window always starts with the first tile
    li(a1, 0)
    jal precacheTiles
    li(a0, window)

beginDrawingRow:
    # clear the MODE_3_FLAG bit
    ori $at, zero, MODE_3_FLAG_CLR
    mtc0 $at, SP_STATUS

    # reset the tile cache
    li($at, tilemapTileCache)
    sh $at, currentTile(zero)

    # check if tilemap is visible
    blez s0, drawWindow

    # a0 pixel x
    # a1 pixel count
    # a2 sprite y
    # a3 src x
    li(a0, 0) # pixel x
    ori a1, s0, 0 # pixel count
    
    lbu a2, (ppuTask + PPUTask_ly)(zero)
    lbu $at, (ppuTask + PPUTask_scy)(zero)
    sub a2, a2, $at
    andi a2, a2, 0x7 # sprite y pos

    lbu a3, (ppuTask + PPUTask_scx)(zero)
    li($at, 8)
    sub a3, $at, a3
    jal copyTileLine
    andi a3, a3, 0x7 # sprite x pos

drawWindow:
    ori a0, s0, 0 # pixel x
    li(a1, GB_SCREEN_WD)
    sub a1, a1, a0 # pixel count
    blez a1, writeOutPixels # check if the window is visible
    andi a2, s1, 0x7 # sprite y pos
    jal copyTileLine
    ori a3, zero, 0 # sprite x pos

writeOutPixels:
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
# a0 - target dma target
# a1 - y line of row
# a2 - tile map flag

precacheLine:
    addi $sp, $sp, -4
    sw return, 0($sp)

    srl a1, a1, 3 # divde by 8 (8 pixels per tile)
    sll a1, a1, 5 # multiply by 32 (32 tiles per row)
    lbu $at, (ppuTask + PPUTask_lcdc)(zero)
    and $at, $at, a2 # determine which tile map to use
    slt $at, zero, $at
    # convert to a tilemap offset
    sll $at, $at, 10
    # store the offset into vram memory
    add a1, $at, a1
    lw $at, (ppuTask + PPUTask_graphicsSource)(zero)
    addi $at, $at, GraphicsMemory_tilemap0
    add a1, $at, a1 # calculate final ram address for tilemap row
    
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

    sll a0, a0, 4 # convert the tile index to a relative tile pointer

    #
    # calculate which tile bank to use
    #
    lbu $at, (tilemapAttrs - tilemap)(a1)
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
# Tiles To Scanline
# a0 pixel x
# a1 pixel count
# a2 sprite y
# a3 src x

copyTileLine:
    # convert y to pixel offset 
    # (2 bytes per row of pixels in a tile)
    sll a2, a2, 1
    # current pixel shift
    addi a3, a3, 7
    andi a3, a3, 7
    # current pixel shift dir
    li(t3, -1)

    # load pointer into tile cache
    lhu t4, currentTile(zero)

copyTileLine_nextPixelRow:
    add $at, t4, a2

    # load the pixel row
    lhu t6, 0($at)

copyTileLine_nextPixel:
    # check if finished copying pixels
    beq a1, zero, copyTileLine_finish    

    srlv $at, t6, a3
    # get current pixel lsb
    andi t1, $at, 0x0100
    # get current pixel msb
    andi $at, $at, 0x0001

    # shift bits into position
    sll $at, $at, 1
    srl t1, t1, 8
    # combine into final pixel value
    or t1, t1, $at

    # write pixel into screen
    sb t1, scanline(a0) 

    # increment screen output
    addi a0, a0, 1

    # calculate next pixel shift amount
    add a3, a3, t3

    # this is a bit of a hack any time
    # a3, the current offset, falls outside
    # the range [0, 7] it should stop copying
    # the current tile. no numbers in [0, 7]
    # have the 4th bit set but both -1 and 8 do
    andi $at, a3, 0x8
    beq $at, zero, copyTileLine_nextPixel
    # decrement pixel count (note delay slot)
    addi a1, a1, -1

    # increment tile cache pointer
    addi t4, t4, GB_TILE_SIZE

    j copyTileLine_nextPixelRow
    andi a3, a3, 7


copyTileLine_finish:
    jr return
    # save current tile before exiting
    sh t4, currentTile(zero)

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