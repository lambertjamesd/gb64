

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