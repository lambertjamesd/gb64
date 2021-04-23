
.data
registerWriteTable:
    # FF00
    .word _GB_WRITE_REG_JOYP        # REG_JOYP
    .word GB_DO_WRITE_NOP           # REG_SERIAL_DATA
    .word  _GB_WRITE_REG_SERIAL     # REG_SERIAL
    .word  GB_DO_WRITE_NOP          

    # FF04
    .word _GB_WRITE_REG_DIV         # REG_DIV
    .word _GB_WRITE_REG_TIMA        # REG_TIMA
    .word _GB_WRITE_REG_TMA         # REG_TMA
    .word _GB_WRITE_REG_TAC         # REG_TAC

    # FF08
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP

    # FF0C
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word _GB_WRITE_REG_INT_REQ     # REG_INTERRUPTS_REQUESTED

    # FF10
    .word _GB_SET_SOUND_REGISTER    # REG_NR10
    .word _GB_SET_SOUND_REGISTER    # REG_NR11
    .word _GB_SET_SOUND_REGISTER    # REG_NR12
    .word _GB_SET_SOUND_REGISTER    # REG_NR13

    # FF14
    .word _GB_SET_SOUND_REGISTER    # REG_NR14
    .word GB_DO_WRITE_NOP
    .word _GB_SET_SOUND_REGISTER    # REG_NR21
    .word _GB_SET_SOUND_REGISTER    # REG_NR22

    # FF18
    .word _GB_SET_SOUND_REGISTER    # REG_NR23
    .word _GB_SET_SOUND_REGISTER    # REG_NR24
    .word _GB_PCM_ENABLE            # REG_NR30
    .word _GB_SET_SOUND_REGISTER    # REG_NR31

    # FF1C
    .word _GB_SET_SOUND_REGISTER    # REG_NR32
    .word _GB_SET_SOUND_REGISTER    # REG_NR33
    .word _GB_SET_SOUND_REGISTER    # REG_NR34
    .word GB_DO_WRITE_NOP
    
    # FF20
    .word _GB_SET_SOUND_REGISTER    # REG_NR41
    .word _GB_SET_SOUND_REGISTER    # REG_NR42
    .word _GB_SET_SOUND_REGISTER    # REG_NR43
    .word _GB_SET_SOUND_REGISTER    # REG_NR44

    # FF24
    .word _GB_BASIC_AUDIO_REGISTER  # REG_NR50
    .word _GB_BASIC_AUDIO_REGISTER  # REG_NR51
    .word _GB_SOUND_ENABLED         # REG_NR52
    .word GB_DO_WRITE_NOP

    # FF28
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP

    # FF2C
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP

    # FF30
    .word _GB_WRITE_REGISTER        # Wave Pattern RAM
    .word _GB_WRITE_REGISTER
    .word _GB_WRITE_REGISTER
    .word _GB_WRITE_REGISTER

    # FF34
    .word _GB_WRITE_REGISTER
    .word _GB_WRITE_REGISTER
    .word _GB_WRITE_REGISTER
    .word _GB_WRITE_REGISTER

    # FF38
    .word _GB_WRITE_REGISTER
    .word _GB_WRITE_REGISTER
    .word _GB_WRITE_REGISTER
    .word _GB_WRITE_REGISTER

    # FF3C
    .word _GB_WRITE_REGISTER
    .word _GB_WRITE_REGISTER
    .word _GB_WRITE_REGISTER
    .word _GB_WRITE_REGISTER

    # FF40
    .word _GB_WRITE_REG_LCDC        # REG_LCDC
    .word _GB_WRITE_REG_LCDC_STATUS # REG_LCDC_STATUS
    .word _GB_WRITE_REGISTER        # REG_SCY
    .word _GB_WRITE_REGISTER        # REG_SCX

    # FF44
    .word GB_DO_WRITE_NOP           # REG_LY
    .word _GB_WRITE_REG_LCY         # REG_LYC
    .word _GB_WRITE_DMA             # REG_DMA
    .word _GB_WRITE_BGP             # REG_BGP

    # FF48
    .word _GB_WRITE_OBP0            # REG_OBP0
    .word _GB_WRITE_OBP1            # REG_OBP1
    .word _GB_WRITE_REGISTER        # REG_WY
    .word _GB_WRITE_REGISTER        # REG_WX

    # FF4C
    .word GB_DO_WRITE_NOP           
    .word _GB_SPEED_KEY1            # REG_KEY1
    .word GB_DO_WRITE_NOP
    .word _GB_WRITE_VBK             # REG_VBK

    # FF50
    .word _GB_WRITE_REG_UNLOAD_BIOS # REG_UNLOAD_BIOS
    .word _GB_WRITE_MASKED_HDMA1    # REG_HDMA1
    .word _GB_WRITE_MASKED_HDMA2    # REG_HDMA2
    .word _GB_WRITE_MASKED_HDMA3    # REG_HDMA3

    # FF54
    .word _GB_WRITE_MASKED_HDMA4    # REG_HDMA4
    .word _GB_START_DMA             # REG_HDMA5
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP

    # FF58
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP

    # FF5C
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP

    # FF60
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP

    # FF64
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP

    # FF68
    .word _GB_WRITE_PALLETE_ADDR_0  # REG_BCPS
    .word _GB_WRITE_PALETTE_0       # REG_BCPD
    .word _GB_WRITE_PALLETE_ADDR_1  # REG_OCPS
    .word _GB_WRITE_PALETTE_1       # REG_OCPD

    # FF6C
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP

    # FF70
    .word _GB_WRITE_SVBK            # REG_SVBK
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP

    # FF74
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    
    # FF78
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP

    # FF7C
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP

.text


_GB_BASIC_REGISTER_WRITE:
    li $at, REG_INTERRUPTS_ENABLED
    beq $at, ADDR,_GB_WRITE_INTERRUPTS_ENABLED
    nop
_GB_WRITE_REGISTER:
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

_GB_BASIC_AUDIO_REGISTER:
    addi $sp, $sp, -16
    sw $ra, 0($sp)
    sw ADDR, 4($sp)
    jal _GB_SYNC_AUDIO
    sw VAL, 8($sp)

    lw $ra, 0($sp)
    lw ADDR, 4($sp)
    lw VAL, 8($sp)

    j _GB_BASIC_REGISTER_WRITE
    addi $sp, $sp, 16

_GB_SET_SOUND_REGISTER:
    save_state_on_stack

    jal GB_CALC_UNSCALED_CLOCKS
    nop

    move $a0, Memory
    move $a1, $v0
    move $a2, ADDR
    move $a3, VAL
    call_c_fn setSoundRegister, 4

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

_GB_WRITE_VBK:
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

######################

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


_GB_WRITE_MASKED_HDMA1:
    j _GB_WRITE_MASKED_HDMA
    li Param0, 0xFF

_GB_WRITE_MASKED_HDMA2:
    j _GB_WRITE_MASKED_HDMA
    li Param0, 0xF0

_GB_WRITE_MASKED_HDMA3:
    j _GB_WRITE_MASKED_HDMA
    li Param0, 0x1F

_GB_WRITE_MASKED_HDMA4:
    j _GB_WRITE_MASKED_HDMA
    li Param0, 0xF0

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


_GB_WRITE_PALETTE_0:
    j _GB_WRITE_PALETTE
    li Param0, 0

_GB_WRITE_PALETTE_1:
    j _GB_WRITE_PALETTE
    li Param0, 1

_GB_WRITE_PALLETE_ADDR_0:
    j _GB_WRITE_PALLETE_ADDR
    li Param0, 0

_GB_WRITE_PALLETE_ADDR_1:
    j _GB_WRITE_PALLETE_ADDR
    li Param0, 1


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

    # intentionally fall through
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

_GB_WRITE_SVBK:
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
    jr $ra
    nop