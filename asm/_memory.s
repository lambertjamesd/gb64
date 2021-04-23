
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
#     andi TMP2, ADDR, 0xFFF

#     # check for sprites
#     slti $at, TMP2, 0xEA0 
#     bnez $at, _GB_BASIC_REGISTER_WRITE

#     # check for unwritable region
#     slti $at, TMP2, 0xF00
#     bnez $at, GB_DO_WRITE_NOP 

#     # check for register ram
#     slti $at, TMP2, 0xF80
#     beqz $at, _GB_BASIC_REGISTER_WRITE 

#     # mask $at into the range 00 - 80
#     andi $at, ADDR, 0xF0
#     srl $at, $at, 1 # get the upper nibble and multiply it by 8

#     la TMP2, _GB_WRITE_JUMP_TABLE
#     add $at, $at, TMP2
#     jr $at
#     nop

# GB_DO_WRITE_REGISTERS_NEW:
    andi TMP2, ADDR, 0xFFF

    # check for sprites
    slti $at, TMP2, 0xEA0 
    bnez $at, _GB_BASIC_REGISTER_WRITE

    # check for unwritable region
    slti $at, TMP2, 0xF00
    bnez $at, GB_DO_WRITE_NOP 

    # check for register ram
    slti $at, TMP2, 0xF80
    beqz $at, _GB_BASIC_REGISTER_WRITE 

    # mask $at into the range 00 - 80
    andi $at, ADDR, 0x7F
    sll $at, $at, 2 # convert to a 4 byte offset
 
    # load jump table offset
    la TMP2, registerWriteTable
    add TMP2, TMP2, $at
    lw $at, 0(TMP2)
    jr $at
    nop

# _GB_WRITE_JUMP_TABLE:
#     j _GB_WRITE_REG_0X
#     nop
#     j _GB_WRITE_SOUND_REG
#     nop
#     j _GB_WRITE_SOUND_REG
#     nop
#     j _GB_WRITE_SOUND_REG
#     nop
#     j _GB_WRITE_REG_4X
#     nop
#     j _GB_WRITE_REG_5X
#     nop
#     j _GB_WRITE_REG_6X
#     nop
#     j _GB_WRITE_REG_7X
#     nop

# ########################

# _GB_WRITE_REG_0X:
#     li $at, REG_JOYP
#     beq ADDR, $at, _GB_WRITE_REG_JOYP
#     li $at, REG_SERIAL
#     beq ADDR, $at, _GB_WRITE_REG_SERIAL
#     li $at, REG_DIV
#     beq ADDR, $at, _GB_WRITE_REG_DIV
#     li $at, REG_TIMA
#     beq ADDR, $at, _GB_WRITE_REG_TIMA
#     li $at, REG_TMA
#     beq ADDR, $at, _GB_WRITE_REG_TMA
#     li $at, REG_TAC
#     beq ADDR, $at, _GB_WRITE_REG_TAC
#     li $at, REG_INTERRUPTS_REQUESTED
#     beq ADDR, $at, _GB_WRITE_REG_INT_REQ
#     nop
#     jr $ra
#     nop


# ############################

# _GB_WRITE_SOUND_REG:
#     li $at, REG_NR52
#     beq $at, ADDR, _GB_SOUND_ENABLED
    
#     read_register_direct $at, REG_NR52
#     andi $at, $at, REG_NR52_ON_OFF
#     beqz $at, _GB_WRITE_SOUND_OFF

#     li $at, REG_NR14
#     beq $at, ADDR, _GB_SET_SOUND_REGISTER
#     li TMP2, 0

#     li $at, REG_NR24
#     beq $at, ADDR, _GB_SET_SOUND_REGISTER
#     li TMP2, 1
    
#     li $at, REG_NR30
#     beq $at, ADDR, _GB_PCM_ENABLE

#     li $at, REG_NR34
#     beq $at, ADDR, _GB_SET_SOUND_REGISTER
#     li TMP2, 2
    
#     li $at, REG_NR44
#     beq $at, ADDR, _GB_SET_SOUND_REGISTER
#     li TMP2, 3

#     li $at, REG_NR50
#     beq $at, ADDR, _GB_BASIC_AUDIO_REGISTER
#     nop
    
#     j _GB_BASIC_REGISTER_WRITE
#     nop

# _GB_WRITE_SOUND_OFF:
#     jr $ra
#     nop

# ############################

# _GB_WRITE_REG_4X:
#     andi $at, ADDR, 0xF
#     sll $at, $at, 3
#     la TMP2, _GB_WRITE_REG_4X_TABLE
#     add $at, $at, TMP2
#     jr $at
#     nop
# _GB_WRITE_REG_4X_TABLE:
#     # LCDC
#     j _GB_WRITE_REG_LCDC
#     nop
#     # LCDC Status
#     j _GB_WRITE_REG_LCDC_STATUS
#     nop
#     # SCY
#     jr $ra
#     write_register_direct VAL, REG_SCY
#     # SCX
#     jr $ra
#     write_register_direct VAL, REG_SCX
#     # LY
#     jr $ra
#     nop
#     # LYC
#     j _GB_WRITE_REG_LCY
#     nop
#     # DMA
#     j _GB_WRITE_DMA
#     nop # TODO
#     # BGP
#     j _GB_WRITE_BGP
#     nop # TODO
#     # OBP0
#      j _GB_WRITE_OBP0
#     nop # TODO
#     # OBP1
#      j _GB_WRITE_OBP1
#     nop # TODO
#     # WY
#     jr $ra
#     write_register_direct VAL, REG_WY
#     # WX
#     jr $ra
#     write_register_direct VAL, REG_WX
#     # Unused
#     jr $ra
#     nop
#     # KEY1
#     j _GB_SPEED_KEY1
#     nop
#     jr $ra
#     nop
#     # VBK
#     andi VAL, VAL, 0x1
#     write_register_direct VAL, REG_VBK
    
#     save_state_on_stack

# .if DEBUG
#     addi $sp, $sp, -8
# .endif

#     move $a0, Memory
#     move $a1, VAL
#     call_c_fn setVRAMBank, 2
#     move $a2, VAL

# .if DEBUG
#     addi $sp, $sp, 8
# .endif

#     restore_state_from_stack
#     jr $ra
#     nop

# ############################

# _GB_WRITE_REG_5X:
#     li $at, REG_UNLOAD_BIOS
#     beq $at, ADDR, _GB_WRITE_REG_UNLOAD_BIOS
    
#     li $at, REG_HDMA1
#     beq $at, ADDR, _GB_WRITE_MASKED_HDMA
#     li Param0, 0xFF
    
#     li $at, REG_HDMA2
#     beq $at, ADDR, _GB_WRITE_MASKED_HDMA
#     li Param0, 0xF0

#     li $at, REG_HDMA3
#     beq $at, ADDR, _GB_WRITE_MASKED_HDMA
#     li Param0, 0x1F

#     li $at, REG_HDMA4
#     beq $at, ADDR, _GB_WRITE_MASKED_HDMA
#     li Param0, 0xF0

#     li $at, REG_HDMA5
#     beq $at, ADDR, _GB_START_DMA

#     nop
#     jr $ra
#     nop

# ############################

# _GB_WRITE_REG_6X:
#     li $at, REG_BCPS
#     beq $at, ADDR, _GB_WRITE_PALLETE_ADDR
#     li Param0, 0

#     li $at, REG_BCPD
#     beq $at, ADDR, _GB_WRITE_PALETTE

#     li $at, REG_OCPS
#     beq $at, ADDR, _GB_WRITE_PALLETE_ADDR
#     li Param0, 1
    
#     li $at, REG_OCPD
#     beq $at, ADDR, _GB_WRITE_PALETTE
#     nop

#     jr $ra
#     nop    

# ############################

# _GB_WRITE_REG_7X:
#     li $at, REG_SVBK
#     bne ADDR, $at, _GB_WRITE_REG_7X_SKIP
#     andi VAL, VAL, 0x7
#     write_register_direct VAL, REG_SVBK

#     save_state_on_stack
# .if DEBUG
#     addi $sp, $sp, -8
# .endif

#     move $a0, Memory
#     move $a1, VAL
#     call_c_fn setInternalRamBank, 2

# .if DEBUG
#     addi $sp, $sp, 8
# .endif

#     restore_state_from_stack
# _GB_WRITE_REG_7X_SKIP:
#     jr $ra
#     nop
    
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
