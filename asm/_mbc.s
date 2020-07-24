
.global mbc3WriteTimer
.align 4
mbc3WriteTimer:
    jr $ra
    nop


.global mbc3ReadTimer
.align 4
mbc3ReadTimer:
    jr $ra
    nop

.global mbc2ReadRam
.align 4
mbc2ReadRam:
    addi $sp, $sp, -8
    jal _GB_DO_READ_RAM
    sw $ra, 0($sp)

    ori $v0, $v0, 0xF0

    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 8
    