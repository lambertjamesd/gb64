
.eqv MBC7_RAM_EEPROM, 0x0
.eqv MBC7_RAM_HAS_DATA, 0x100
.eqv MBC7_RAM_ACCEL_X, 0x104
.eqv MBC7_RAM_ACCEL_Y, 0x106
.eqv MBC7_RAM_SENSOR_X, 0x108
.eqv MBC7_RAM_SENSOR_Y, 0x10A
.eqv MBC7_RAM_DATA_IN, 0x10C
.eqv MBC7_RAM_DATA_OUT, 0x110
.eqv MBC7_RAM_DATA_COUNT, 0x114

.data
MBC7_WRITE_JUMP_TABLE:
    .word _GB_WRITE_MBC7_ERASE
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

_GB_WRITE_MBC7_ERASE:
    li $at, 0x55
    bne $at, VAL, GB_DO_WRITE_NOP

    lw TMP2, (MEMORY_ADDR_TABLE + 4 * 0xA)(Memory)
    sw $zero, MBC7_RAM_HAS_DATA(TMP2)
    li $at, 0x8000
    sw $at, MBC7_RAM_ACCEL_X(TMP2)
    jr $ra
    sw $at, MBC7_RAM_ACCEL_Y(TMP2)

_GB_WRITE_MBC7_LATCH:
    li $at, 0xAA
    bne $at, VAL, GB_DO_WRITE_NOP
    
    lw TMP2, (MEMORY_ADDR_TABLE + 4 * 0xA)(Memory)
    lw $at, MBC7_RAM_HAS_DATA(TMP2)
    bne $at, $zero, GB_DO_WRITE_NOP

    sw VAL, MBC7_RAM_HAS_DATA(TMP2)
    lw $at, MBC7_RAM_SENSOR_X(TMP2)
    sw $at, MBC7_RAM_ACCEL_X(TMP2)
    lw $at, MBC7_RAM_SENSOR_Y(TMP2)
    jr $ra
    sw $at, MBC7_RAM_ACCEL_Y(TMP2)

_GB_WRITE_MBC7_EEPROM:
    jr $ra
    nop

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
    jr $ra
    nop
