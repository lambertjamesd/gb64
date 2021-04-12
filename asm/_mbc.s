
.global mbc3WriteTimer
.align 4
mbc3WriteTimer:
    lw TMP2, MEMORY_MISC_RAM_BANK(Memory)
    slti $at, TMP2, 0x8
    bnez $at, mbc3WriteTimer_error
    slti $at, TMP2, 0xD
    beqz $at, mbc3WriteTimer_error
    nop
    add TMP2, TMP2, Memory
    sb VAL, (REG_RTC_S+MEMORY_MISC_START-MM_REGISTER_START-0x8)(TMP2)

    # set tmp2 to 0
    dsubu TMP2, TMP2, TMP2
    # load upper day bits
    read_register_direct TMP2, REG_RTC_DH
    andi TMP2, TMP2, 0x81
    sll TMP2, TMP2, 8
    # load lower day bits
    read_register_direct $at, REG_RTC_DL
    or TMP2, TMP2, $at
    # days to hours
    li $at, 24
    multu TMP2, $at
    # hours to minutes
    read_register_direct $at, REG_RTC_H
    mflo TMP2
    add TMP2, TMP2, $at
    li $at, 60
    multu TMP2, $at
    # minutes to second 
    read_register_direct $at, REG_RTC_M
    mflo TMP2
    add TMP2, TMP2, $at
    li $at, 60
    multu TMP2, $at
    # seconds to microseconds
    dsll TMP2, TMP2, 20
    sd TMP2, MEMORY_MISC_TIMER(Memory)
    jr $ra
    nop

mbc3WriteTimer_error:
    jr $ra
    nop


.global mbc3ReadTimer
.align 4
mbc3ReadTimer:
    lw TMP2, MEMORY_MISC_RAM_BANK(Memory)
    slti $at, TMP2, 0x8
    bnez $at, mbc3ReadTimer_error
    slti $at, TMP2, 0xD
    beqz $at, mbc3ReadTimer_error
    nop
    add TMP2, TMP2, Memory
    jr $ra
    lbu $v0, (REG_RTC_S+MEMORY_MISC_START-MM_REGISTER_START-0x8)(TMP2)

mbc3ReadTimer_error:
    jr $ra
    li $v0, 0xFF