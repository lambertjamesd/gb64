
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
    ori $at, $zero, MM_REGISTER_START
    sub $at, ADDR, $at 
    bgez $at, GB_DO_WRITE_REGISTERS_CALL # if ADDR >= 0xFE00 do register logic

    ori $at, $zero, 0x8000
    sub $at, ADDR, $at
    bgez $at, _GB_DO_WRITE # if ADDR >= 0x8000 just write
    nop 
    j _GB_CALL_WRITE_CALLBACK # call bank switching callback
    lw Param0, MEMORY_BANK_SWITCHING(Memory)
_GB_DO_WRITE:
    srl $at, ADDR, 12 # load bank in $at
    andi ADDR, ADDR, 0xFFF # keep offset in ADDR
    sll $at, $at, 2 # word align the memory map offset
    add $at, $at, Memory # lookup start of bank in array at Memory
    lw $at, 0($at) # load start of memory bank
    add ADDR, ADDR, $at # use address relative to memory bank
    jr $ra
    sb VAL, 0(ADDR) # store the byte

.eqv _WRITE_CALLBACK_FRAME_SIZE, 0x28

_GB_CALL_WRITE_CALLBACK:
    save_state_on_stack

    move $a0, Memory
    move $a1, ADDR
    jalr $ra, Param0
    move $a2, VAL

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
    jr $ra # TODO
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
    addi $sp, $sp, -4
    sw $ra, 0($sp)
    jal CHECK_FOR_INTERRUPT # check if an interrupt should be called
    write_register_direct VAL, REG_INTERRUPTS_ENABLED # set requested interrupts
    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 4

########################

_GB_WRITE_REG_0X:
    ori $at, $zero, REG_JOYP
    beq ADDR, $at, _GB_WRITE_REG_JOYP
    ori $at, $zero, REG_DIV
    beq ADDR, $at, _GB_WRITE_REG_DIV
    ori $at, $zero, REG_TIMA
    beq ADDR, $at, _GB_WRITE_REG_TIMA
    ori $at, $zero, REG_TMA
    beq ADDR, $at, _GB_WRITE_REG_TMA
    ori $at, $zero, REG_TAC
    beq ADDR, $at, _GB_WRITE_REG_TAC
    ori $at, $zero, REG_INTERRUPTS_REQUESTED
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
    
_GB_WRITE_REG_DIV:
    # DIV = (((CYCLES_RUN << 2) + _REG_DIV_OFFSET) >> 8) & 0xFFFF
    # _REG_DIV_OFFSET = -(CYCLES_RUN << 2) & 0xFFFF
    sll $at, CYCLES_RUN, 2
    sub $at, $zero, $at
    jr $ra
    write_register16_direct $at, _REG_DIV_OFFSET
    
_GB_WRITE_REG_TIMA:
    addi $sp, $sp, -4
    sw $ra, 0($sp)
    jal REMOVE_STOPPING_POINT
    li Param0, CPU_STOPPING_POINT_TYPE_TIMER_RESET
    lw $ra, 0($sp)
    addi $sp, $sp, 4
    j CALCULATE_NEXT_TIMER_INTERRUPT
    write_register_direct VAL, REG_TIMA
    
_GB_WRITE_REG_TMA:
    jr $ra
    write_register_direct VAL, REG_TMA

_GB_WRITE_REG_TAC:
    addi $sp, $sp, -4
    sw $ra, 0($sp)
    jal CALCULATE_TIMA_VALUE
    nop
    jal REMOVE_STOPPING_POINT
    li Param0, CPU_STOPPING_POINT_TYPE_TIMER_RESET
    lw $ra, 0($sp)
    addi $sp, $sp, 4
    j CALCULATE_NEXT_TIMER_INTERRUPT
    write_register_direct VAL, REG_TAC
    
_GB_WRITE_REG_INT_REQ:
    addi $sp, $sp, -4
    sw $ra, 0($sp)
    jal CHECK_FOR_INTERRUPT # check if an interrupt should be called
    write_register_direct VAL, REG_INTERRUPTS_REQUESTED # set requested interrupts
    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 4


############################

_GB_WRITE_SOUND_REG:
    li $at, REG_NR14
    beq $at, ADDR, _GB_RESTART_SOUND
    li TMP2, 0

    li $at, REG_NR24
    beq $at, ADDR, _GB_RESTART_SOUND
    li TMP2, 1
    
    li $at, REG_NR34
    beq $at, ADDR, _GB_RESTART_SOUND
    li TMP2, 2
    
    li $at, REG_NR44
    beq $at, ADDR, _GB_RESTART_SOUND
    li TMP2, 3

    j _GB_BASIC_REGISTER_WRITE
    nop

_GB_RESTART_SOUND:
    andi $at, VAL, 0x80
    beqz $at, _GB_BASIC_REGISTER_WRITE # only restart sound if upper bit is set
    nop

    save_state_on_stack

    # write the register before making the call
    jal _GB_BASIC_REGISTER_WRITE

    lui Param0, %hi(restartSound)
    addiu Param0, Param0, %lo(restartSound)

    move $a0, Memory
    move $a1, CYCLES_RUN
    jalr $ra, Param0
    # TODO use unscaledCyclesRun
    move $a2, TMP2

    restore_state_from_stack

    jr $ra
    nop

_GB_SYNC_AUDIO:
    save_state_on_stack

    move $a0, Memory
    call_c_fn tickAudio
    # TODO use unscaledCyclesRun
    move $a1, CYCLES_RUN

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
    sll VAL, VAL, 13 # mulitply by 0x8000 (VRAM bank size)
    li $at, MEMORY_VRAM
    add $at, $at, Memory
    add $at, $at, VAL
    sw $at, (MEMORY_ADDR_TABLE + 4 * MEMORY_VRAM_BANK_INDEX)(Memory)
    addi $at, $at, 0x1000
    jr $ra
    sw $at, (MEMORY_ADDR_TABLE + 4 * (MEMORY_VRAM_BANK_INDEX + 1))(Memory)

_GB_WRITE_REG_LCDC:
    read_register_direct $at, REG_LCDC
    xor $at, VAL, $at
    andi $at, $at, REG_LCDC_LCD_ENABLE
    beqz $at, _GB_WRITE_REG_LCDC_WRITE # if the LCD status didn't change do nothing
    andi $at, VAL, REG_LCDC_LCD_ENABLE
    addi $sp, $sp, -4
    beqz $at, _GB_WRITE_REG_LCDC_OFF
    sw $ra, 0($sp)

_GB_WRITE_REG_LCDC_ON:

    read_register_direct $at, REG_LCDC_STATUS
    andi $at, $at, %lo(~REG_LCDC_STATUS_MODE)
    ori $at, $at, REG_LCDC_STATUS_MODE_2
    write_register_direct $at, REG_LCDC_STATUS # set mode to 2

    # set up next stopping point to be the current cycle
    # with LY at the last line to update the current
    # lcd state on DECODE_NEXT
    li $at, GB_SCREEN_LINES - 1
    write_register_direct $at, REG_LY # set LY to 0

    sll TMP2, CYCLES_RUN, 8
    jal QUEUE_STOPPING_POINT
    addi TMP2, TMP2, CPU_STOPPING_POINT_TYPE_SCREEN_2
      
    lw $ra, 0($sp)
    j _GB_WRITE_REG_LCDC_WRITE
    addi $sp, $sp, 4 

_GB_WRITE_REG_LCDC_OFF:
    
    read_register_direct $at, REG_LCDC_STATUS
    andi $at, $at, %lo(~(REG_LCDC_STATUS_MODE | REG_LCDC_STATUS_LYC))
    ori $at, $at, REG_LCDC_STATUS_MODE_1
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
    lw $ra, 0($sp)
    addi $sp, $sp, 4 
_GB_WRITE_REG_LCDC_WRITE:
    jr $ra
    write_register_direct VAL, REG_LCDC
    
############################

_GB_WRITE_REG_LCDC_STATUS:
    addi $sp, $sp, -4
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
    addi $sp, $sp, -4
    sw $ra, 0($sp)
    
    read_register_direct Param0, REG_LCDC_STATUS
    jal CHECK_LCDC_STAT_FLAG
    read_register_direct TMP3, REG_LY

    # write LYC
    write_register_direct VAL, REG_LYC

    # todo LYC flags
    andi Param0, Param0, %lo(REG_LCDC_STATUS_LYC)
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
    addi $sp, $sp, 4 
    
############################

_GB_WRITE_DMA:
    andi $at, VAL, 0xF0 # calculate memory bank address offset
    srl $at, $at, 2
    add $at, Memory, $at # offset from memory bank start
    lw ADDR, 0($at) # load the memory bank
    sll TMP2, VAL, 8 # calculate offset from bank
    andi TMP2, TMP2, 0xFFF # mask away upper bits
    add ADDR, ADDR, TMP2 # calucate starting point inside memory bank
    addi TMP3, Memory, MEMORY_MISC_START # load start of misc memory
    li TMP2, 0xA0 # loop once per sprite
_GB_DMA_LOOP:
    lw $at, 0(ADDR)
    sw $at, 0(TMP3)
    addi TMP2, TMP2, -4 # decrease counter
    addi ADDR, ADDR, 4 # increase read address
    bnez TMP2, _GB_DMA_LOOP # loop until zero
    addi TMP3, TMP3, 4 # increase write address
    jr $ra
    nop

############################

_GB_SPEED_KEY1:
    read_register_direct $at, REG_KEY1
    addi $at, $at, 0xFE
    andi VAL, VAL, 0x01
    or VAL, VAL, $at
    jr $ra
    write_register_direct VAL, REG_KEY1
    
############################

_GB_PALLETE_COLORS:
    .data
    .align 4
    .word 0b1110011111110101, 0b1000111000011101, 0b0011001101010101, 0b0000100011001001

_GB_WRITE_BGP:
    la $at, MEMORY_BG_PAL
    jr $ra
    add ADDR, Memory, $at

_GB_WRITE_OBP0:
    la $at, MEMORY_OBJ_PAL
    jr $ra
    add ADDR, Memory, $at

_GB_WRITE_OBP1:
    la $at, MEMORY_OBJ_PAL + 8 # second pallete index
    jr $ra
    add ADDR, Memory, $at

_GB_WRITE_DMA_PAL:
    la TMP2, _GB_PALLETE_COLORS

    andi $at, VAL, 0x03
    sll $at, $at, 2
    add $at, TMP2, $at
    lw $at, 0($at)
    sh $at, 0(ADDR)

    andi $at, VAL, 0x0C
    add $at, TMP2, $at
    lw $at, 0($at)
    sh $at, 2(ADDR)

    andi $at, VAL, 0x30
    srl $at, $at, 2
    add $at, TMP2, $at
    lw $at, 0($at)
    sh $at, 4(ADDR)

    andi $at, VAL, 0xC0
    srl $at, $at, 4
    add $at, TMP2, $at
    lw $at, 0($at)
    jr $ra
    sh $at, 4(ADDR)



############################

_GB_WRITE_REG_5X:
    li $at, REG_UNLOAD_BIOS
    beq $at, ADDR, _GB_WRITE_REG_UNLOAD_BIOS
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
    sw PC_MEMORY_BANK, 0x1C($sp)
    sw $ra, 0x20($sp)
    sw $fp, 0x24($sp)

    call_c_fn unloadBIOS
    move $a0, Memory

    lhu GB_PC, 0x8($sp)
    lhu GB_SP, 0xA($sp)

    lw CYCLES_RUN, 0xC($sp)
    lw CPUState, 0x10($sp)
    lw Memory, 0x14($sp)
    lw CycleTo, 0x18($sp)
    lw PC_MEMORY_BANK, 0x1C($sp)
    lw $ra, 0x20($sp)
    lw $fp, 0x24($sp)

    # TODO different initial values in GBC mode
    li GB_A, 0x01
    li GB_F, 0xB0
    li GB_B, 0x00
    li GB_C, 0x13
    li GB_D, 0x00
    li GB_E, 0xD8
    li GB_H, 0x01
    li GB_L, 0x4D

    jr $ra
    addi $sp, $sp, _WRITE_CALLBACK_FRAME_SIZE

    
############################

_GB_WRITE_REG_7X:
    ori $at, $zero, REG_SVBK
    bne ADDR, $at, _GB_WRITE_REG_7X_SKIP
    andi VAL, VAL, 0x7
    write_register_direct VAL, REG_SVBK
    bne VAL, $zero, _GB_WRITE_REG_7X_CHANGE_BANK
    nop
    ori VAL, VAL, 0x1
_GB_WRITE_REG_7X_CHANGE_BANK:
    sll $at, VAL, 12 # 
    add $at, $at, Memory
    add $at, $at, MEMORY_RAM_START
    jr $ra
    sw $at, (MEMORY_ADDR_TABLE + 4 * MEMORY_RAM_BANK_INDEX)(Memory)
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
    ori $at, $zero, MM_REGISTER_START
    sub $at, ADDR, $at
    bgez $at, GB_DO_READ_REGISTERS # if ADDR >= 0xFE00

    srl $at, ADDR, 12 # load bank in $at
    andi ADDR, ADDR, 0xFFF # keep offset in ADDR
    sll $at, $at, 2 # word align the memory map offset
    add $at, $at, Memory # lookup start of bank in array at Memory
    lw $at, 0($at) # load start of memory bank
    add ADDR, ADDR, $at # use address relative to memory bank
    jr $ra
    lbu $v0, 0(ADDR) # load the byte

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
    beq $at, ADDR, _GB_DO_READ_SOUND_ENABLE
    nop

    li $at, MEMORY_MISC_START-MM_REGISTER_START
    add ADDR, $at, ADDR # ADDR relative to MISC memory
    add ADDR, Memory, ADDR # Relative to memory
    jr $ra
    lbu $v0, 0(ADDR)


_GB_DO_READ_SOUND_ENABLE:
    addi $sp, $sp, -4
    sw $ra, 0($sp)

    jal _GB_SYNC_AUDIO
    nop

    read_register_direct $v0, REG_NR52

    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 4
    