
.eqv MBC7_RAM_EEPROM, 0x0
.eqv MBC7_RAM_HAS_DATA, 0x100
.eqv MBC7_RAM_ACCEL_X, 0x104
.eqv MBC7_RAM_ACCEL_Y, 0x106
.eqv MBC7_RAM_SENSOR_X, 0x108
.eqv MBC7_RAM_SENSOR_Y, 0x10A
.eqv MBC7_RAM_DATA_IN, 0x10C
.eqv MBC7_RAM_DATA_OUT, 0x110
.eqv MBC7_RAM_DATA_COUNT, 0x114
.eqv MBC7_RAM_CURRENT_EEPROM, 0x118
.eqv MBC7_RAM_WRITE_ENABLE, 0x119

.eqv MBC7_EEPROM_DO, 0x1
.eqv MBC7_EEPROM_DI, 0x2
.eqv MBC7_EEPROM_CLOCK, 0x40
.eqv MBC7_EEPROM_CS, 0x80

.data
MBC7_WRITE_JUMP_TABLE:
    .word _GB_WRITE_MBC7_ACCEL_ERASE
    .word _GB_WRITE_MBC7_LATCH
    # x data
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    # y data
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    # z data?
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP

    .word _GB_WRITE_MBC7_EEPROM
    .word GB_DO_WRITE_NOP

    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP
    .word GB_DO_WRITE_NOP

MBC7_READ_JUMP_TABLE:
    .word GB_DO_READ_NOP
    .word GB_DO_READ_NOP

    .word _GB_READ_MBC7_ACCEL_X_LOW
    .word _GB_READ_MBC7_ACCEL_X_HIGH

    .word _GB_READ_MBC7_ACCEL_Y_LOW
    .word _GB_READ_MBC7_ACCEL_Y_HIGH

    .word _GB_READ_MBC7_ACCEL_Z_LOW
    .word GB_DO_READ_NOP

    .word _GB_READ_MBC7_EEPROM
    .word GB_DO_READ_NOP

    .word GB_DO_READ_NOP
    .word GB_DO_READ_NOP

    .word GB_DO_READ_NOP
    .word GB_DO_READ_NOP

    .word GB_DO_READ_NOP
    .word GB_DO_READ_NOP

.text

.global GB_DO_WRITE_MBC7
.align 4
GB_DO_WRITE_MBC7:
    andi $at, ADDR, 0xF0
    srl $at, $at, 2
    lui TMP2, %hi(MBC7_WRITE_JUMP_TABLE)
    add $at, $at, TMP2
    lw TMP2, %lo(MBC7_WRITE_JUMP_TABLE)($at)
    jr TMP2 
    nop

_GB_WRITE_MBC7_ACCEL_ERASE:
    li $at, 0x55
    bne $at, VAL, GB_DO_WRITE_NOP

    lw TMP2, (MEMORY_ADDR_TABLE + 4 * 0xA)(Memory)
    sw $zero, MBC7_RAM_HAS_DATA(TMP2)
    li $at, 0x8000
    sh $at, MBC7_RAM_ACCEL_X(TMP2)
    jr $ra
    sh $at, MBC7_RAM_ACCEL_Y(TMP2)

_GB_WRITE_MBC7_LATCH:
    li $at, 0xAA
    bne $at, VAL, GB_DO_WRITE_NOP
    
    lw TMP2, (MEMORY_ADDR_TABLE + 4 * 0xA)(Memory)
    lw $at, MBC7_RAM_HAS_DATA(TMP2)
    bne $at, $zero, GB_DO_WRITE_NOP

    sw VAL, MBC7_RAM_HAS_DATA(TMP2)
    lhu $at, MBC7_RAM_SENSOR_X(TMP2)
    sh $at, MBC7_RAM_ACCEL_X(TMP2)
    lhu $at, MBC7_RAM_SENSOR_Y(TMP2)
    jr $ra
    sh $at, MBC7_RAM_ACCEL_Y(TMP2)

_GB_WRITE_MBC7_EEPROM:
    lw TMP2, (MEMORY_ADDR_TABLE + 4 * 0xA)(Memory)

    lbu TMP3, MBC7_RAM_CURRENT_EEPROM(TMP2)

    # prevent the data out bit from being written
    andi $at, VAL, 0xC2
    andi TMP4, TMP3, 0x1
    or TMP4, $at, TMP4
    sb TMP4, MBC7_RAM_CURRENT_EEPROM(TMP2)

    # check for clock cycle
    xori $at, TMP3, MBC7_EEPROM_CLOCK
    and $at, $at, VAL
    andi $at, $at, MBC7_EEPROM_CLOCK
    beqz $at, _GB_WRITE_MBC7_FINISH

    # check for output bit
    lw TMP3, MBC7_RAM_DATA_OUT(TMP2)
    li $at, -1
    beq $at, TMP3, _GB_WRITE_MBC7_SAVE_DO
    ori TMP4, TMP4, MBC7_EEPROM_DO

    # get the byte index of the memory
    srl $at, TMP3, 3 
    andi $at, $at, 0xFF
    add $at, $at, TMP2
    lbu $at, MBC7_RAM_EEPROM($at)

    # get the bit index in the byte
    andi TMP5, TMP3, 0x7

    #increment output bit
    addi TMP3, TMP3, 1
    sw TMP3, MBC7_RAM_DATA_OUT(TMP2)

    # mask out specific bit in byte
    li TMP3, 0x80
    # shift mask into position
    srlv TMP3, TMP3, TMP5
    and $at, $at, TMP3
    # set output according to bit read
    beqz $at, _GB_WRITE_MBC7_SAVE_DO
    andi TMP4, TMP4, (0xFF ^ MBC7_EEPROM_DO)
    ori TMP4, TMP4, MBC7_EEPROM_DO
_GB_WRITE_MBC7_SAVE_DO:
    sb TMP4, MBC7_RAM_CURRENT_EEPROM(TMP2)

_GB_WRITE_MBC7_CHECK_INPUT:
    lw TMP3, MBC7_RAM_DATA_COUNT(TMP2)
    li $at, -1
    beq $at, TMP3, _GB_WRITE_MBC7_CHECK_START
    # increment bit count
    addi TMP3, TMP3, 1

    # save new bit count
    sw TMP3, MBC7_RAM_DATA_COUNT(TMP2)

    # shift in current bit
    lw TMP5, MBC7_RAM_DATA_IN(TMP2)
    sll TMP5, TMP5, 1
    andi $at, TMP4, MBC7_EEPROM_DI
    srl $at, $at, 1
    or TMP5, TMP5, $at
    sw TMP5, MBC7_RAM_DATA_IN(TMP2)

    li $at, 10
    beq $at, TMP3, _GB_WRITE_MBC7_CHECK_10_BITS
    li $at, 26
    beq $at, TMP3, _GB_WRITE_MBC7_CHECK_26_BITS
    nop

_GB_WRITE_MBC7_FINISH:
    jr $ra
    nop

_GB_WRITE_MBC7_CHECK_10_BITS:
    andi TMP3, TMP5, 0x300
    li $at, 0x200
    beq $at, TMP3, _GB_WRITE_MBC7_READ
    li $at, 0x300
    beq $at, TMP3, _GB_WRITE_MBC7_ERASE

    andi TMP3, TMP5, 0x3C0
    li $at, 0x0C0
    beq $at, TMP3, _GB_WRITE_MBC7_EWEN
    li $at, 0x080
    beq $at, TMP3, _GB_WRITE_MBC7_ERAL
    li $at, 0x000
    beq $at, TMP3, _GB_WRITE_MBC7_EWDS
    nop

    j _GB_WRITE_MBC7_FINISH
    nop

_GB_WRITE_MBC7_CHECK_26_BITS:
    srl TMP3, TMP5, 16
    andi TMP3, TMP3, 0x3C0
    li $at, 0x040
    beq $at, TMP3, _GB_WRITE_MBC7_WRAL

    andi TMP3, TMP3, 0x300
    li $at, 0x100
    beq $at, TMP3, _GB_WRITE_MBC7_WRITE
    nop

.if DEBUG
    # should never reach here
    teq $zero, $zero
.endif
    j _GB_WRITE_MBC7_FINISH
    nop

_GB_WRITE_MBC7_READ:
    # calculate read address
    andi $at, TMP5, 0x7F
    # convert short address to bit address
    sll $at, $at, 4
    sw $at, MBC7_RAM_DATA_OUT(TMP2)

    sw $zero, MBC7_RAM_DATA_IN(TMP2)
    addi $at, $zero, -1
    j _GB_WRITE_MBC7_FINISH
    sw $at, MBC7_RAM_DATA_COUNT(TMP2)

_GB_WRITE_MBC7_ERASE:
    # check if writing is enabled
    lbu $at, MBC7_RAM_WRITE_ENABLE(TMP2)
    beqz $at, _GB_WRITE_MBC7_EEPROM_RESET
    
    # get erase address
    andi $at, TMP5, 0x7F
    sll $at, $at, 1
    add $at, $at, TMP2
    # erase at address
    li TMP3, 0xFFFF
    j _GB_WRITE_MBC7_EEPROM_RESET
    sh TMP3, MBC7_RAM_EEPROM($at)

_GB_WRITE_MBC7_EWEN:
    # enable writing
    li $at, 1
    j _GB_WRITE_MBC7_EEPROM_RESET
    sb $at, MBC7_RAM_WRITE_ENABLE(TMP2)

_GB_WRITE_MBC7_ERAL:
    # check if writing is enabled
    lbu $at, MBC7_RAM_WRITE_ENABLE(TMP2)
    beqz $at, _GB_WRITE_MBC7_EEPROM_RESET

    li $at, -1

_GB_WRITE_MBC7_FILL_MEM:
    move TMP4, TMP2
    li TMP3, 0x100

_GB_WRITE_MBC7_FILL_MEM_NEXT:
    sw $at, MBC7_RAM_EEPROM(TMP4)
    addi TMP3, TMP3, -4
    bgtz TMP3, _GB_WRITE_MBC7_FILL_MEM_NEXT
    addi TMP4, TMP4, 4

    j _GB_WRITE_MBC7_EEPROM_RESET
    nop


_GB_WRITE_MBC7_EWDS:
    # disable writing
    j _GB_WRITE_MBC7_EEPROM_RESET
    sb $zero, MBC7_RAM_WRITE_ENABLE(TMP2)

_GB_WRITE_MBC7_WRAL:
    # check if writing is enabled
    lbu $at, MBC7_RAM_WRITE_ENABLE(TMP2)
    beqz $at, _GB_WRITE_MBC7_EEPROM_RESET
    nop
    # fill memory with lower 16 bits
    j _GB_WRITE_MBC7_FILL_MEM
    andi $at, TMP5, 0xFFFF

_GB_WRITE_MBC7_WRITE:
    # check if writing is enabled
    lbu $at, MBC7_RAM_WRITE_ENABLE(TMP2)
    beqz $at, _GB_WRITE_MBC7_EEPROM_RESET

    # calculate write address
    srl TMP3, TMP5, 15
    andi TMP3, TMP3, 0xFE
    add TMP3, TMP3, TMP2

    # calculate write value
    andi $at, TMP5, 0xFFFF

    #write value
    j _GB_WRITE_MBC7_EEPROM_RESET
    sh $at, MBC7_RAM_EEPROM(TMP3)
    

_GB_WRITE_MBC7_CHECK_START:
    andi $at, TMP4, MBC7_EEPROM_CS | MBC7_EEPROM_DI
    xori $at, $at, MBC7_EEPROM_CS | MBC7_EEPROM_DI
    bnez $at, _GB_WRITE_MBC7_FINISH
    nop
    j _GB_WRITE_MBC7_FINISH
    sw $zero, MBC7_RAM_DATA_COUNT(TMP2)

_GB_WRITE_MBC7_EEPROM_RESET:
    addi $at, $zero, -1
    sw $at, MBC7_RAM_DATA_OUT(TMP2)
    sw $zero, MBC7_RAM_DATA_IN(TMP2)
    jr $ra
    sw $at, MBC7_RAM_DATA_COUNT(TMP2)

.global GB_DO_READ_MBC7
.align 4
GB_DO_READ_MBC7:
    andi $at, ADDR, 0xF0
    srl $at, $at, 2
    lui TMP2, %hi(MBC7_READ_JUMP_TABLE)
    add $at, $at, TMP2
    lw TMP2, %lo(MBC7_READ_JUMP_TABLE)($at)
    jr TMP2
    nop

_GB_READ_MBC7_ACCEL_X_LOW:
    lw TMP2, (MEMORY_ADDR_TABLE + 4 * 0xA)(Memory)
    jr $ra
    lbu $v0, (MBC7_RAM_ACCEL_X + 1)(TMP2)
    
_GB_READ_MBC7_ACCEL_X_HIGH:
    lw TMP2, (MEMORY_ADDR_TABLE + 4 * 0xA)(Memory)
    jr $ra
    lbu $v0, (MBC7_RAM_ACCEL_X)(TMP2)

_GB_READ_MBC7_ACCEL_Y_LOW:
    lw TMP2, (MEMORY_ADDR_TABLE + 4 * 0xA)(Memory)
    jr $ra
    lbu $v0, (MBC7_RAM_ACCEL_Y + 1)(TMP2)

_GB_READ_MBC7_ACCEL_Y_HIGH:
    lw TMP2, (MEMORY_ADDR_TABLE + 4 * 0xA)(Memory)
    jr $ra
    lbu $v0, (MBC7_RAM_ACCEL_Y)(TMP2)

_GB_READ_MBC7_ACCEL_Z_LOW:
    jr $ra
    li $v0, 0

_GB_READ_MBC7_EEPROM:
    lw TMP2, (MEMORY_ADDR_TABLE + 4 * 0xA)(Memory)
    jr $ra
    lbu $v0, MBC7_RAM_CURRENT_EEPROM(TMP2)
