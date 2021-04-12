
#######################
# Reads the address HL into $v0
#######################

READ_HL:
    sll ADDR, GB_H, 8 # load high order bits
    j GB_DO_READ
    or ADDR, ADDR, GB_L # load low order bits

######################
# Writes 16 bit VAL to ADDR
# stomps on VAL, ADDR, TMP2
######################

GB_DO_WRITE_16:
    la $ra, DECODE_NEXT
GB_DO_WRITE_16_CALL:
    addi $sp, $sp, -8
    sh VAL, 0($sp)
    sh ADDR, 2($sp)
    sw $ra, 4($sp)
    jal GB_DO_WRITE_CALL
    andi VAL, VAL, 0xFF

    lhu VAL, 0($sp)
    lhu ADDR, 2($sp)

    srl VAL, VAL, 8
    addi ADDR, ADDR, 1

    jal GB_DO_WRITE_CALL
    andi ADDR, ADDR, 0xFFFF

    lw $ra, 4($sp)
    jr $ra
    addi $sp, $sp, 8

######################
# Writes VAL to ADDR
# stomps on VAL, ADDR, TMP2
######################

GB_DO_WRITE:
    la $ra, DECODE_NEXT
GB_DO_WRITE_CALL:
    # read which bank to use
    andi $at, ADDR, 0xF000
    # shift bank to be a memory offset
    srl $at, $at, 10
    # calculate read target
    add $at, $at, Memory
    # load jump target
    lw $at, MEMORY_WRITE_TABLE($at)
    # jump to address
    jr $at
    nop


.global GB_DO_WRITE_NOP
.align 4
GB_DO_WRITE_NOP:
    jr $ra
    nop

.global GB_DO_WRITE_MBC2
.align 4
GB_DO_WRITE_MBC2:
    andi $v0, ADDR, 0x1FF
    lw $at, (MEMORY_ADDR_TABLE + 0xA * 4)(Memory)
    add $at, $at, $v0
    jr $ra
    sb VAL, 0($at)

.global GB_DO_WRITE_FF
.align 4
GB_DO_WRITE_FF:
    ori $at, $zero, MM_REGISTER_START
    sub $at, ADDR, $at 
    bgez $at, GB_DO_WRITE_REGISTERS_CALL # if ADDR >= 0xFE00 do register logic
_GB_DO_WRITE:
    srl $at, ADDR, 12 # load bank in $at

    xori TMP2, $at, 0xA
    andi TMP2, TMP2, 0xE
    bne $at, TMP2, _GB_DO_WRITE_RAM
    sll $at, $at, 2 # word align the memory map offset

    lw TMP2, MEMORY_CART_WRITE(Memory)
    beqz TMP2, _GB_DO_WRITE_RAM
    nop

    jr TMP2
    nop

_GB_DO_WRITE_RAM:
    andi ADDR, ADDR, 0xFFF # keep offset in ADDR
    add $at, $at, Memory # lookup start of bank in array at Memory
    lw $at, 0($at) # load start of memory bank
    add ADDR, ADDR, $at # use address relative to memory bank
    jr $ra
    sb VAL, 0(ADDR) # store the byte

.eqv _WRITE_CALLBACK_FRAME_SIZE, 0x28

.global GB_WRITE_ROM_BANK
.align 4
GB_WRITE_ROM_BANK:
    lw Param0, MEMORY_BANK_SWITCHING(Memory)
    save_state_on_stack

.if DEBUG
    addi $sp, $sp, -16
.endif

    move $a0, Memory
    move $a1, ADDR
    jalr $ra, Param0
    move $a2, VAL

.if DEBUG
    addi $sp, $sp, 16
.endif

    restore_state_from_stack
    jr $ra
    nop


######################
# Writes VAL to ADDR in the range 0xFE00-0xFFFF
######################

GB_DO_WRITE_REGISTERS:
    la $ra, DECODE_NEXT
GB_DO_WRITE_REGISTERS_CALL:
    li $at, -0xFF00
    add $at, $at, ADDR # ADDR relative to MISC memory
    bltz $at, _GB_BASIC_REGISTER_WRITE # just write the sprite memory bank
    addi $at, $at, -0x80
    bgez $at, _GB_BASIC_REGISTER_WRITE # just write memory above interrupt table

    addi $at, $at, 0x80 # move $at back into range 0x00 - 0x80
    andi $at, $at, 0xF0
    srl $at, $at, 1 # get the upper nibble and multiply it by 8

    la TMP2, _GB_WRITE_JUMP_TABLE
    add $at, $at, TMP2
    jr $at
    nop

_GB_WRITE_JUMP_TABLE:
    j _GB_WRITE_REG_0X
    nop
    j _GB_WRITE_SOUND_REG
    nop
    j _GB_WRITE_SOUND_REG
    nop
    j _GB_WRITE_SOUND_REG
    nop
    j _GB_WRITE_REG_4X
    nop
    j _GB_WRITE_REG_5X
    nop
    j _GB_WRITE_REG_6X
    nop
    j _GB_WRITE_REG_7X
    nop

_GB_BASIC_REGISTER_WRITE:
    li $at, REG_INTERRUPTS_ENABLED
    beq $at, ADDR,_GB_WRITE_INTERRUPTS_ENABLED
    nop

    li $at, MEMORY_MISC_START-MM_REGISTER_START
    add ADDR, $at, ADDR # ADDR relative to MISC memory
    add ADDR, Memory, ADDR # Relative to memory
    jr $ra
    sb VAL, 0(ADDR)

_GB_WRITE_INTERRUPTS_ENABLED:
    addi $sp, $sp, -8
    sw $ra, 0($sp)
    jal CHECK_FOR_INTERRUPT # check if an interrupt should be called
    write_register_direct VAL, REG_INTERRUPTS_ENABLED # set requested interrupts
    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 8

########################

_GB_WRITE_REG_0X:
    li $at, REG_JOYP
    beq ADDR, $at, _GB_WRITE_REG_JOYP
    li $at, REG_SERIAL
    beq ADDR, $at, _GB_WRITE_REG_SERIAL
    li $at, REG_DIV
    beq ADDR, $at, _GB_WRITE_REG_DIV
    li $at, REG_TIMA
    beq ADDR, $at, _GB_WRITE_REG_TIMA
    li $at, REG_TMA
    beq ADDR, $at, _GB_WRITE_REG_TMA
    li $at, REG_TAC
    beq ADDR, $at, _GB_WRITE_REG_TAC
    li $at, REG_INTERRUPTS_REQUESTED
    beq ADDR, $at, _GB_WRITE_REG_INT_REQ
    nop
    jr $ra
    nop

_GB_WRITE_REG_JOYP:
    ori VAL, VAL, 0xF # set all buttons to high
    andi $at, VAL, 0x10
    bnez $at, _GB_WRITE_REG_JOYP_NEXT
    read_register_direct TMP2, _REG_JOYSTATE
    ori $at, TMP2, 0xF0
    and VAL, VAL, $at # mask some bits low
_GB_WRITE_REG_JOYP_NEXT:
    andi $at, VAL, 0x20
    bnez $at, _GB_WRITE_REG_JOYP_FINISH
    srl $at, TMP2, 4
    ori $at, $at, 0xF0
    and VAL, VAL, $at # mask some bits low
_GB_WRITE_REG_JOYP_FINISH:
    jr $ra
    write_register_direct VAL, REG_JOYP 

_GB_WRITE_REG_SERIAL:
    addi $sp, $sp, -8
    sw $ra, 0($sp)
    andi $at, VAL, REG_SERIAL_TRANSFER
    beqz $at, _GB_WRITE_REG_SERIAL_END
    nop
    jal REMOVE_STOPPING_POINT
    li Param0, CPU_STOPPING_POINT_SERIAL_RECIEVE

    andi $at, VAL, REG_SERIAL_CLOCK
    beqz $at, _GB_WRITE_REG_SERIAL_END

    li TMP2, 1024
    lbu $at, CPU_STATE_GBC(CPUState)
    beqz $at, _GB_WRITE_REG_SERIAL_SCHEDULE
    
    andi $at, VAL, REG_SERIAL_SPEED
    beqz $at, _GB_WRITE_REG_SERIAL_CHECK_SPEED
    nop
    srl TMP2, TMP2, 1

_GB_WRITE_REG_SERIAL_CHECK_SPEED:
    read_register_direct $at, REG_KEY1
    andi $at, $at, REG_KEY1_CURRENT_SPEED
    beqz $at, _GB_WRITE_REG_SERIAL_SCHEDULE
    nop
    srl TMP2, TMP2, 1
_GB_WRITE_REG_SERIAL_SCHEDULE:
    add TMP2, TMP2, CYCLES_RUN
    sll TMP2, TMP2, 8
    jal QUEUE_STOPPING_POINT
    addi TMP2, TMP2, CPU_STOPPING_POINT_SERIAL_RECIEVE
_GB_WRITE_REG_SERIAL_END:
    write_register_direct VAL, REG_SERIAL
    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 8

    
_GB_WRITE_REG_DIV:
    # DIV = (((CYCLES_RUN << 2) + _REG_DIV_OFFSET) >> 8) & 0xFFFF
    # _REG_DIV_OFFSET = -(CYCLES_RUN << 2) & 0xFFFF
    sll $at, CYCLES_RUN, 2
    sub $at, $zero, $at
    write_register16_direct $at, _REG_DIV_OFFSET
    addi $sp, $sp, -8
    sw $ra, 0($sp)
    jal REMOVE_STOPPING_POINT
    li Param0, CPU_STOPPING_POINT_TYPE_TIMER_RESET
    lw $ra, 0($sp)
    j CALCULATE_NEXT_TIMER_INTERRUPT
    addi $sp, $sp, 8
    
_GB_WRITE_REG_TIMA:
    addi $sp, $sp, -8
    sw $ra, 0($sp)
    jal REMOVE_STOPPING_POINT
    li Param0, CPU_STOPPING_POINT_TYPE_TIMER_RESET
    lw $ra, 0($sp)
    addi $sp, $sp, 8
    j CALCULATE_NEXT_TIMER_INTERRUPT
    write_register_direct VAL, REG_TIMA
    
_GB_WRITE_REG_TMA:
    jr $ra
    write_register_direct VAL, REG_TMA

_GB_WRITE_REG_TAC:
    addi $sp, $sp, -8
    sw $ra, 0($sp)
    jal CALCULATE_TIMA_VALUE
    nop
    jal REMOVE_STOPPING_POINT
    li Param0, CPU_STOPPING_POINT_TYPE_TIMER_RESET
    lw $ra, 0($sp)
    addi $sp, $sp, 8
    j CALCULATE_NEXT_TIMER_INTERRUPT
    write_register_direct VAL, REG_TAC
    
_GB_WRITE_REG_INT_REQ:
    addi $sp, $sp, -8
    sw $ra, 0($sp)
    ori VAL, VAL, 0xE0 # upper 3 bits are always 1
    jal CHECK_FOR_INTERRUPT # check if an interrupt should be called
    write_register_direct VAL, REG_INTERRUPTS_REQUESTED # set requested interrupts
    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 8


############################

_GB_WRITE_SOUND_REG:
    li $at, REG_NR52
    beq $at, ADDR, _GB_SOUND_ENABLED
    li TMP2, 3
    
    read_register_direct $at, REG_NR52
    andi $at, $at, REG_NR52_ON_OFF
    beqz $at, _GB_WRITE_SOUND_OFF

    li $at, REG_NR14
    beq $at, ADDR, _GB_RESTART_SOUND
    li TMP2, 0

    li $at, REG_NR24
    beq $at, ADDR, _GB_RESTART_SOUND
    li TMP2, 1
    
    li $at, REG_NR30
    beq $at, ADDR, _GB_PCM_ENABLE

    li $at, REG_NR34
    beq $at, ADDR, _GB_RESTART_SOUND
    li TMP2, 2
    
    li $at, REG_NR44
    beq $at, ADDR, _GB_RESTART_SOUND
    li TMP2, 3
    
    j _GB_BASIC_REGISTER_WRITE
    nop

_GB_WRITE_SOUND_OFF:
    jr $ra
    nop


_GB_RESTART_SOUND:
    andi $at, VAL, 0x80
    beqz $at, _GB_BASIC_REGISTER_WRITE # only restart sound if upper bit is set
    nop

    save_state_on_stack

    # write the register before making the call
    jal _GB_BASIC_REGISTER_WRITE
    nop

    jal GB_CALC_UNSCALED_CLOCKS
    nop

    move $a0, Memory
    move $a1, $v0
    # TODO use unscaledCyclesRun
    move $a2, TMP2
    call_c_fn restartSound, 3

    restore_state_from_stack

    jr $ra
    nop

_GB_PCM_ENABLE:
    addi $sp, $sp, -8
    sw $ra, 0($sp)
    sb VAL, 4($sp)
    jal _GB_SYNC_AUDIO
    nop
    lbu VAL, 4($sp)
    write_register_direct VAL, REG_NR30
    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 8


_GB_SOUND_ENABLED:
    addi $sp, $sp, -8
    sw $ra, 0($sp)
    sb VAL, 4($sp)
    jal _GB_SYNC_AUDIO
    nop
    lbu VAL, 4($sp)
    andi VAL, VAL, REG_NR52_ON_OFF
    read_register_direct $at, REG_NR52
    andi $at, $at, %lo(~REG_NR52_ON_OFF)
    or VAL, VAL, $at
    write_register_direct VAL, REG_NR52
    andi $at, VAL, REG_NR52_ON_OFF
    bnez $at, _GB_SOUND_ENABLED_END
    nop
    write_register_direct $zero, REG_NR10 + 0
    write_register_direct $zero, REG_NR10 + 1
    write_register_direct $zero, REG_NR10 + 2
    write_register_direct $zero, REG_NR10 + 3
    write_register_direct $zero, REG_NR10 + 4

    write_register_direct $zero, REG_NR10 + 5
    write_register_direct $zero, REG_NR10 + 6
    write_register_direct $zero, REG_NR10 + 7
    write_register_direct $zero, REG_NR10 + 8
    write_register_direct $zero, REG_NR10 + 9
    
    write_register_direct $zero, REG_NR10 + 10
    write_register_direct $zero, REG_NR10 + 11
    write_register_direct $zero, REG_NR10 + 12
    write_register_direct $zero, REG_NR10 + 13
    write_register_direct $zero, REG_NR10 + 14
    
    write_register_direct $zero, REG_NR10 + 15
    write_register_direct $zero, REG_NR10 + 16
    write_register_direct $zero, REG_NR10 + 17
    write_register_direct $zero, REG_NR10 + 18
    write_register_direct $zero, REG_NR10 + 19
    
    write_register_direct $zero, REG_NR10 + 20
    write_register_direct $zero, REG_NR10 + 21
    write_register_direct $zero, REG_NR10 + 22

_GB_SOUND_ENABLED_END:
    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 8

_GB_SYNC_AUDIO:
    save_state_on_stack

    jal GB_CALC_UNSCALED_CLOCKS
    nop

    move $a0, Memory
    move $a1, $v0
    call_c_fn tickAudio, 2

    restore_state_from_stack

    jr $ra
    nop

############################

_GB_WRITE_REG_4X:
    andi $at, ADDR, 0xF
    sll $at, $at, 3
    la TMP2, _GB_WRITE_REG_4X_TABLE
    add $at, $at, TMP2
    jr $at
    nop
_GB_WRITE_REG_4X_TABLE:
    # LCDC
    j _GB_WRITE_REG_LCDC
    nop
    # LCDC Status
    j _GB_WRITE_REG_LCDC_STATUS
    nop
    # SCY
    jr $ra
    write_register_direct VAL, REG_SCY
    # SCX
    jr $ra
    write_register_direct VAL, REG_SCX
    # LY
    jr $ra
    nop
    # LYC
    j _GB_WRITE_REG_LCY
    nop
    # DMA
    j _GB_WRITE_DMA
    nop # TODO
    # BGP
    j _GB_WRITE_BGP
    nop # TODO
    # OBP0
     j _GB_WRITE_OBP0
    nop # TODO
    # OBP1
     j _GB_WRITE_OBP1
    nop # TODO
    # WY
    jr $ra
    write_register_direct VAL, REG_WY
    # WX
    jr $ra
    write_register_direct VAL, REG_WX
    # Unused
    jr $ra
    nop
    # KEY1
    j _GB_SPEED_KEY1
    nop
    jr $ra
    nop
    # VBK
    andi VAL, VAL, 0x1
    write_register_direct VAL, REG_VBK
    
    save_state_on_stack

.if DEBUG
    addi $sp, $sp, -8
.endif

    move $a0, Memory
    move $a1, VAL
    call_c_fn setVRAMBank, 2
    move $a2, VAL

.if DEBUG
    addi $sp, $sp, 8
.endif

    restore_state_from_stack
    jr $ra
    nop

_GB_WRITE_REG_LCDC:
    read_register_direct $at, REG_LCDC
    xor $at, VAL, $at
    andi $at, $at, REG_LCDC_LCD_ENABLE
    beqz $at, _GB_WRITE_REG_LCDC_WRITE # if the LCD status didn't change do nothing
    andi $at, VAL, REG_LCDC_LCD_ENABLE
    addi $sp, $sp, -8
    beqz $at, _GB_WRITE_REG_LCDC_OFF
    sw $ra, 0($sp)

_GB_WRITE_REG_LCDC_ON:

    read_register_direct $at, REG_LCDC_STATUS
    andi $at, $at, %lo(~REG_LCDC_STATUS_MODE)
    ori $at, $at, REG_LCDC_STATUS_MODE_1
    write_register_direct $at, REG_LCDC_STATUS # set mode to 1

    # setting status to 1 with ly = 0
    # will result in the screen timing starting
    # correctly
    write_register_direct $zero, REG_LY # set LY to 0

    sll TMP2, CYCLES_RUN, 8
    jal QUEUE_STOPPING_POINT
    addi TMP2, TMP2, CPU_STOPPING_POINT_TYPE_SCREEN_2
      
    lw $ra, 0($sp)
    j _GB_WRITE_REG_LCDC_WRITE
    addi $sp, $sp, 8 

_GB_WRITE_REG_LCDC_OFF:
    
    read_register_direct $at, REG_LCDC_STATUS
    andi $at, $at, %lo(~(REG_LCDC_STATUS_MODE | REG_LCDC_STATUS_LYC))
    # todo check if LYC is 0
    write_register_direct $at, REG_LCDC_STATUS # set mode to 1
    write_register_direct $zero, REG_LY # set LY to 0
    jal REMOVE_STOPPING_POINT # update stopping point
    li Param0, CPU_STOPPING_POINT_TYPE_SCREEN_0
    jal REMOVE_STOPPING_POINT # update stopping point
    li Param0, CPU_STOPPING_POINT_TYPE_SCREEN_1
    jal REMOVE_STOPPING_POINT # update stopping point
    li Param0, CPU_STOPPING_POINT_TYPE_SCREEN_2
    jal REMOVE_STOPPING_POINT # update stopping point
    li Param0, CPU_STOPPING_POINT_TYPE_SCREEN_3
    jal REMOVE_STOPPING_POINT # update stopping point
    li Param0, CPU_STOPPING_POINT_TYPE_SCREEN_FINAL_LINE
    lw $ra, 0($sp)
    addi $sp, $sp, 8 
_GB_WRITE_REG_LCDC_WRITE:
    jr $ra
    write_register_direct VAL, REG_LCDC
    
############################

_GB_WRITE_REG_LCDC_STATUS:
    addi $sp, $sp, -8
    sw $ra, 0($sp)

    read_register_direct Param0, REG_LCDC_STATUS
    jal CHECK_LCDC_STAT_FLAG
    read_register_direct TMP3, REG_LY

    # prevent writing readonly registers
    andi VAL, VAL, %lo(~(REG_LCDC_STATUS_MODE | REG_LCDC_STATUS_LYC))
    andi $at, Param0, (REG_LCDC_STATUS_MODE | REG_LCDC_STATUS_LYC)
    or VAL, VAL, $at

    write_register_direct VAL, REG_LCDC_STATUS
    j _GB_CHECK_RISING_STAT
    move Param0, VAL

############################

_GB_WRITE_REG_LCY:
    addi $sp, $sp, -8
    sw $ra, 0($sp)
    
    read_register_direct Param0, REG_LCDC_STATUS
    jal CHECK_LCDC_STAT_FLAG
    read_register_direct TMP3, REG_LY

    # write LYC
    write_register_direct VAL, REG_LYC

    # todo LYC flags
    andi Param0, Param0, %lo(~REG_LCDC_STATUS_LYC)
    bne TMP3, VAL, _GB_WRITE_REG_LCY_SKIP_COMP_SET
    nop
    # write LCDC status back
    ori Param0, Param0, REG_LCDC_STATUS_LYC
_GB_WRITE_REG_LCY_SKIP_COMP_SET:
    write_register_direct Param0, REG_LCDC_STATUS

_GB_CHECK_RISING_STAT:
    jal CHECK_LCDC_STAT_FLAG
    move $v1, $v0

    # look for rising STAT_FLAG
    slt $v1, $v1, $v0
    beqz $v1, _GB_CHECK_RISING_STAT_FINISH
    nop

    jal REQUEST_INTERRUPT
    li VAL, INTERRUPT_LCD_STAT

_GB_CHECK_RISING_STAT_FINISH:
    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 8 
    
############################

_GB_WRITE_DMA:
    addi $sp, $sp, -8
    sw $ra, 0($sp)
    # cancel any existing DMA
    jal REMOVE_STOPPING_POINT
    li Param0, CPU_STOPPING_POINT_TYPE_DMA

    write_register_direct VAL, REG_DMA # save DMA addresss
    write_register_direct $zero, _REG_DMA_CURRENT # restart dma copy
    addi $at, CYCLES_RUN, CYCLES_PER_INSTR
    write_register16_direct $at, _REG_DMA_LAST_CYCLE # start copy 1 cycle from now

    addi TMP2, CYCLES_RUN, CYCLES_PER_INSTR * 2
    sll TMP2, TMP2, 8
    addi TMP2, TMP2, CPU_STOPPING_POINT_TYPE_DMA # first byte finsihes 2 cycles from now
    lw $ra, 0($sp)
    j QUEUE_STOPPING_POINT
    addi $sp, $sp, 8 

############################

_GB_SPEED_KEY1:
    read_register_direct $at, REG_KEY1
    andi $at, $at, %lo(~REG_KEY1_PREPARE_SWITCH)
    andi VAL, VAL, REG_KEY1_PREPARE_SWITCH
    or VAL, VAL, $at
    jr $ra
    write_register_direct VAL, REG_KEY1
    
############################

    .data
    .align 2
.global gBGPColors
gBGPColors:
    .half 0b1110011111110101, 0b1000111000011101, 0b0011001101010101, 0b0000100011001001
.global gOBP0Colors
gOBP0Colors:
    .half 0b1110011111110101, 0b1000111000011101, 0b0011001101010101, 0b0000100011001001
.global gOBP1Colors
gOBP1Colors:
    .half 0b1110011111110101, 0b1000111000011101, 0b0011001101010101, 0b0000100011001001

    .align 4
.global gPalleteDirty
gPalleteDirty:
    .word 1
.global gCyclesWaitingForMode0
gCyclesWaitingForMode0:
    .word 0
    .text

_GB_WRITE_BGP:
    write_register_direct VAL, REG_BGP
    la $at, MEMORY_BG_PAL
    la TMP2, gBGPColors
    j _GB_WRITE_DMA_PAL
    add ADDR, Memory, $at

_GB_WRITE_OBP0:
    write_register_direct VAL, REG_OBP0
    la $at, MEMORY_OBJ_PAL
    la TMP2, gOBP0Colors
    j _GB_WRITE_DMA_PAL
    add ADDR, Memory, $at

_GB_WRITE_OBP1:
    write_register_direct VAL, REG_OBP1
    la $at, MEMORY_OBJ_PAL + 8 # second palette index
    la TMP2, gOBP1Colors
    j _GB_WRITE_DMA_PAL
    add ADDR, Memory, $at

_GB_WRITE_DMA_PAL:
    lbu $at, CPU_STATE_GBC(CPUState)
    bnez $at, _GB_WRITE_DMA_PAL_SKIP

    andi $at, VAL, 0x03
    sll $at, $at, 1
    add $at, TMP2, $at
    lhu $at, 0($at)
    sh $at, 0(ADDR)

    andi $at, VAL, 0x0C
    srl $at, $at, 1
    add $at, TMP2, $at
    lhu $at, 0($at)
    sh $at, 2(ADDR)

    andi $at, VAL, 0x30
    srl $at, $at, 3
    add $at, TMP2, $at
    lhu $at, 0($at)
    sh $at, 4(ADDR)

    andi $at, VAL, 0xC0
    srl $at, $at, 5
    add $at, TMP2, $at
    lhu $at, 0($at)
    sh $at, 6(ADDR)

    la $at, gPalleteDirty
    li TMP2, 1
    jr $ra
    sw TMP2, 0($at)

_GB_WRITE_DMA_PAL_SKIP:
    jr $ra
    nop


############################

_GB_WRITE_REG_5X:
    li $at, REG_UNLOAD_BIOS
    beq $at, ADDR, _GB_WRITE_REG_UNLOAD_BIOS
    
    li $at, REG_HDMA1
    beq $at, ADDR, _GB_WRITE_MASKED_HDMA
    li Param0, 0xFF
    
    li $at, REG_HDMA2
    beq $at, ADDR, _GB_WRITE_MASKED_HDMA
    li Param0, 0xF0

    li $at, REG_HDMA3
    beq $at, ADDR, _GB_WRITE_MASKED_HDMA
    li Param0, 0x1F

    li $at, REG_HDMA4
    beq $at, ADDR, _GB_WRITE_MASKED_HDMA
    li Param0, 0xF0

    li $at, REG_HDMA5
    beq $at, ADDR, _GB_START_DMA

    nop
    jr $ra
    nop

_GB_WRITE_REG_UNLOAD_BIOS:
    addi $sp, $sp, -_WRITE_CALLBACK_FRAME_SIZE
    sh GB_PC, 0x8($sp)
    sh GB_SP, 0xA($sp)

    sw CYCLES_RUN, 0xC($sp)
    sw CPUState, 0x10($sp)
    sw Memory, 0x14($sp)
    sw CycleTo, 0x18($sp)
    sw $ra, 0x20($sp)
    sw $fp, 0x24($sp)

    read_register_direct $at, REG_UNLOAD_BIOS
    bnez $at, _GB_WRITE_REG_UNLOAD_BIOS_SKIP
    li $at, 1
    write_register_direct $at, REG_UNLOAD_BIOS

    move $a0, Memory
    call_c_fn unloadBIOS, 1

    lhu GB_SP, 0xA($sp)

    lw CYCLES_RUN, 0xC($sp)
    lw CPUState, 0x10($sp)
    lw Memory, 0x14($sp)
    lw CycleTo, 0x18($sp)
    
    jal SET_GB_PC
    lhu Param0, 0x8($sp)

    lw $ra, 0x20($sp)
    lw $fp, 0x24($sp)


    lbu $at, CPU_STATE_GBC(CPUState)
    bnez $at, _GB_WRITE_REG_UNLOAD_BIOS_CGB

    li GB_A, 0x01
    li GB_F, 0xB0
    li GB_B, 0x00
    li GB_C, 0x13
    li GB_D, 0x00
    li GB_E, 0xD8
    li GB_H, 0x01
    li GB_L, 0x4D
    li GB_SP, 0xFFFE

    jr $ra
    addi $sp, $sp, _WRITE_CALLBACK_FRAME_SIZE

_GB_WRITE_REG_UNLOAD_BIOS_CGB:
    li GB_A, 0x11
    li GB_F, 0x80
    li GB_B, 0x00
    li GB_C, 0x00
    li GB_D, 0xFF
    li GB_E, 0x56
    li GB_H, 0x00
    li GB_L, 0x0D
    li GB_SP, 0xFFFE

_GB_WRITE_REG_UNLOAD_BIOS_SKIP:
    jr $ra
    addi $sp, $sp, _WRITE_CALLBACK_FRAME_SIZE

_GB_WRITE_MASKED_HDMA:
    j _GB_BASIC_REGISTER_WRITE
    and VAL, VAL, Param0

_GB_START_DMA:
    lbu $at, CPU_STATE_GBC(CPUState)
    beqz $at, _GB_SKIP_DMA
    andi $at, VAL, 0x7F
    write_register_direct $at, REG_HDMA5
    andi $at, VAL, 0x80
    bnez $at, _GB_SKIP_DMA
    nop
    addi CYCLES_RUN, CYCLES_RUN, 1
    addi $sp, $sp, -8
    sw $ra, 0($sp)
_GB_DMA_LOOP:
    jal GB_DMA_BLOCK
    nop
    beqz $v0, _GB_DMA_LOOP
    nop
    lw $ra, 0($sp)
    addi $sp, $sp, 8
_GB_SKIP_DMA:
    jr $ra
    nop

GB_DMA_BLOCK:
    read_register_direct $at, REG_KEY1
    andi $at, $at, REG_KEY1_CURRENT_SPEED
    beqz $at, _GB_DMA_BLOCK_START
    addi CYCLES_RUN, CYCLES_RUN, 8
    addi CYCLES_RUN, CYCLES_RUN, 8
_GB_DMA_BLOCK_START:
    read_register_direct $at, REG_HDMA1
    sll TMP2, $at, 8
    read_register_direct $at, REG_HDMA2
    or TMP2, TMP2, $at

    addi TMP2, TMP2, 0x10
    andi $at, TMP2, 0xFF
    write_register_direct $at, REG_HDMA2
    srl $at, TMP2, 8
    write_register_direct $at, REG_HDMA1
    addi TMP2, TMP2, -0x10
    
    read_register_direct $at, REG_HDMA3
    sll TMP3, $at, 8
    read_register_direct $at, REG_HDMA4
    or TMP3, TMP3, $at

    addi TMP3, TMP3, 0x10
    andi $at, TMP3, 0xFF
    write_register_direct $at, REG_HDMA4
    srl $at, TMP3, 8
    write_register_direct $at, REG_HDMA3
    addi TMP3, TMP3, -0x10

    andi $at, TMP2, 0xF000
    srl $at, 10
    add $at, $at, Memory
    lw $at, MEMORY_ADDR_TABLE($at)
    andi TMP2, TMP2, 0xFF0
    add TMP2, TMP2, $at

    addi TMP3, TMP3, 0x8000
    andi $at, TMP3, 0xF000
    srl $at, 10
    add $at, $at, Memory
    lw $at, MEMORY_ADDR_TABLE($at)
    andi TMP3, TMP3, 0xFF0
    add TMP3, TMP3, $at

    lw $at, 0(TMP2)
    sw $at, 0(TMP3)
    lw $at, 4(TMP2)
    sw $at, 4(TMP3)
    lw $at, 8(TMP2)
    sw $at, 8(TMP3)
    lw $at, 12(TMP2)
    sw $at, 12(TMP3)

    read_register_direct TMP2, REG_HDMA5
    andi $at, TMP2, 0x7F
    beqz $at, _GB_DMA_BLOCK_DONE

    addi TMP2, TMP2, -1
    write_register_direct TMP2, REG_HDMA5

    jr $ra
    li $v0, 0
_GB_DMA_BLOCK_DONE:
    li $at, 0xFF # value when DMA is done
    write_register_direct $at, REG_HDMA5
    jr $ra
    li $v0, 1

############################

_GB_WRITE_REG_6X:
    li $at, REG_BCPS
    beq $at, ADDR, _GB_WRITE_PALLETE_ADDR
    li Param0, 0

    li $at, REG_BCPD
    beq $at, ADDR, _GB_WRITE_PALETTE

    li $at, REG_OCPS
    beq $at, ADDR, _GB_WRITE_PALLETE_ADDR
    li Param0, 1
    
    li $at, REG_OCPD
    beq $at, ADDR, _GB_WRITE_PALETTE
    nop

    jr $ra
    nop    

_GB_WRITE_PALETTE:
    la $at, gPalleteDirty
    li TMP2, 1
    sw TMP2, 0($at)

    sll TMP3, Param0, 1
    add TMP3, TMP3, Memory # TMP3 used to point to memory register

    li $at, MEMORY_BG_PAL
    sll TMP2, Param0, 6
    add TMP2, TMP2, $at # TMP2 used to point to palette
    add TMP2, TMP2, Memory

    lbu $at, (MEMORY_MISC_START+REG_BCPS-MM_REGISTER_START)(TMP3)
    andi $at, $at, 0x3F
    add TMP2, TMP2, $at
    sb VAL, 0(TMP2)
    
    lbu $at, (MEMORY_MISC_START+REG_BCPS-MM_REGISTER_START)(TMP3)
    andi $at, $at, 0x80
    beqz $at, _GB_BASIC_REGISTER_WRITE
    nop
    lbu VAL, (MEMORY_MISC_START+REG_BCPS-MM_REGISTER_START)(TMP3)
    addi ADDR, ADDR, -1
    addi VAL, VAL, 1
    andi VAL, VAL, 0xBF

_GB_WRITE_PALLETE_ADDR:
    sll TMP3, Param0, 1
    add TMP3, TMP3, Memory # TMP3 used to point to memory register
    
    sb VAL, (MEMORY_MISC_START+REG_BCPS-MM_REGISTER_START)(TMP3)
    
    li $at, MEMORY_BG_PAL
    sll TMP2, Param0, 6
    add TMP2, TMP2, $at # TMP2 used to point to palette
    add TMP2, TMP2, Memory
    andi $at, VAL, 0x3F
    add TMP2, TMP2, $at

    lbu $at, 0(TMP2)

    jr $ra
    sb $at, (MEMORY_MISC_START+REG_BCPD-MM_REGISTER_START)(TMP3)

    
############################

_GB_WRITE_REG_7X:
    li $at, REG_SVBK
    bne ADDR, $at, _GB_WRITE_REG_7X_SKIP
    andi VAL, VAL, 0x7
    write_register_direct VAL, REG_SVBK

    save_state_on_stack
.if DEBUG
    addi $sp, $sp, -8
.endif

    move $a0, Memory
    move $a1, VAL
    call_c_fn setInternalRamBank, 2

.if DEBUG
    addi $sp, $sp, 8
.endif

    restore_state_from_stack
_GB_WRITE_REG_7X_SKIP:
    jr $ra
    nop
    
######################
# Reads ADDR into $v0
######################

GB_DO_READ_16:
    addi $sp, $sp, -8
    sw $ra, 0($sp)
    jal GB_DO_READ
    sh ADDR, 4($sp)

    sh $v0, 6($sp)
    lhu ADDR, 4($sp)
    addi ADDR, ADDR, 1
    jal GB_DO_READ
    andi ADDR, ADDR, 0xFFFF

    lhu $at, 6($sp)
    lw $ra, 0($sp)
    addi $sp, $sp, 8

    sll $v0, $v0, 8
    jr $ra
    or $v0, $v0, $at
    
######################
# Reads ADDR into $v0
######################

GB_DO_READ:
    # read which bank to use
    andi $at, ADDR, 0xF000
    # shift bank to be a memory offset
    srl $at, $at, 10
    # calculate read target
    add $at, $at, Memory
    # load jump target
    lw $at, MEMORY_READ_TABLE($at)
    # jump to address
    jr $at
    nop

.global GB_DO_READ_NOP
.align 4
GB_DO_READ_NOP:
    jr $ra
    li $v0, 0xFF

.global GB_DO_READ_MBC2
.align 4
GB_DO_READ_MBC2:
    andi $v0, ADDR, 0x1FF
    lw $at, (MEMORY_ADDR_TABLE + 0xA * 4)(Memory)
    add $at, $at, $v0
    lbu $v0, 0($at)
    jr $ra
    # mask out upper nibble
    ori $v0, $v0, 0xF0

######################
# Reads the last bank of memory 0xF000-0xFFFF
######################
.global GB_DO_READ_FF
.align 4
GB_DO_READ_FF:
    ori $at, $zero, MM_REGISTER_START
    sub $at, ADDR, $at
    bgez $at, GB_DO_READ_REGISTERS # if ADDR >= 0xFE00

    srl $at, ADDR, 12 # load bank in $at
_GB_DO_READ_RAM:
    add $at, $at, Memory # lookup start of bank in array at Memory
    lw $at, 0($at) # load start of memory bank
    andi $v0, ADDR, 0xFFF # keep offset in ADDR
    add $at, $v0, $at # use address relative to memory bank
    jr $ra
    lbu $v0, 0($at) # load the byte

######################
# Reads the ADDR Value in the range 0xFE00-0xFFFF
######################

GB_DO_READ_REGISTERS:
    # check for DIV recalculation
    li $at, REG_DIV
    beq $at, ADDR, CALCULATE_DIV_VALUE

    # check for TIMA recalculation
    li $at, REG_TIMA
    beq $at, ADDR, CALCULATE_TIMA_VALUE

    li $at, REG_NR52
    beq $at, ADDR, _GB_DO_READ_SOUND
    nop

    li $at, REG_NR10
    slt $at, ADDR, $at
    bnez $at, _GB_DO_READ_REGISTERS_DIRECT

    li $at, REG_LCDC
    slt $at, ADDR, $at
    bnez $at, _GB_DO_READ_SOUND
    nop

_GB_DO_READ_REGISTERS_DIRECT:
    li $at, MEMORY_MISC_START-MM_REGISTER_START
    add $at, $at, ADDR # ADDR relative to MISC memory
    add $at, Memory, $at # Relative to memory
    jr $ra
    lbu $v0, 0($at)

_GB_DO_READ_SOUND:
    addi $sp, $sp, -8
    sw $ra, 0($sp)
    sh ADDR, 4($sp)

    jal _GB_SYNC_AUDIO
    nop

    lhu ADDR, 4($sp)

    la $at, (_SOUND_READ_MASKS - REG_NR10)
    add $at, $at, ADDR
    jal _GB_DO_READ_REGISTERS_DIRECT
    lbu TMP2, 0($at)

    or $v0, $v0, TMP2

    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 8
    

.data    
_SOUND_READ_MASKS:
     .byte 0x80,0x3F,0x00,0xFF,0xBF                     # NR10-NR15
     .byte 0xFF,0x3F,0x00,0xFF,0xBF                     # NR20-NR25
     .byte 0x7F,0xFF,0x9F,0xFF,0xBF                     # NR30-NR35
     .byte 0xFF,0xFF,0x00,0x00,0xBF                     # NR40-NR45
     .byte 0x00,0x00,0x70                               # NR50-NR52
     .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
     .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00      # Wave RAM
     .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
.text
