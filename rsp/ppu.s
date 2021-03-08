
#include "./ostask.s"
#include "./registers.s"
#include "./pputask.s"
#include "./gameboy.s"

.name zero, $0
.name return, $31

.name p0, $20
.name p1, $21
.name p2, $22
.name p3, $23

.name tmp, $1
    
.data

.align 8
ppuTask:
    .space PPUTask_sizeof

.align 8
scanline:
    .space GB_SCREEN_WD

    .dmax 4032

.text 0x0

Main:
    # ori p0, zero, 0
    # lw p1, osTask_ucode_data($0)
    # lw p2, osTask_ucode_data_size($0)
    # jal DMAproc
    # ori p3, zero, 0

    ori p0, zero, ppuTask
    lw p1, osTask_data_ptr($0)
    lw p2, osTask_data_ptr_size($0)
    jal DMAproc
    ori p3, zero, 0

    jal DMAWait
    # note delay slot

.name index, $2
.name output, $3
.name workingMemory, $4
.name pixelIndex, $5
    ori index, zero, GB_SCREEN_WD
    lw output, (ppuTask + PPUTask_output)($0)
    ori workingMemory, zero, scanline
    ori pixelIndex, zero, 0

xFillLoop:
    sw pixelIndex, 0(workingMemory)
    addi index, index, -1
    addi workingMemory, workingMemory, 1
    addi pixelIndex, pixelIndex, 1
    bne index, zero, xFillLoop
    andi pixelIndex, pixelIndex, 3

    ori index, zero, GB_SCREEN_HT
yFillLoop:
    # ori p0, workingMemory, 0
    # ori p1, output, 0
    # ori p2, zero, GB_SCREEN_WD
    # jal DMAproc
    # ori p3, zero, 1
    
    # addi index, index, -1
    # bne index, zero, yFillLoop
    # addi output, output, GB_SCREEN_WD

    # jal DMAWait
    # nop

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
    mfc0 tmp, DMA_BUSY
    bne tmp, zero, DMAWait
    nop
    jr return
    nop