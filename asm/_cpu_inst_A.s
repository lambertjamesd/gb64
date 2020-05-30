
### 0xAX
GB_AND_B:
    and GB_A, GB_A, GB_B
    bne GB_A, $zero, DECODE_NEXT
    li GB_F, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_AND_C:
    and GB_A, GB_A, GB_C
    bne GB_A, $zero, DECODE_NEXT
    li GB_F, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_AND_D:
    and GB_A, GB_A, GB_D
    bne GB_A, $zero, DECODE_NEXT
    li GB_F, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_AND_E:
    and GB_A, GB_A, GB_E
    bne GB_A, $zero, DECODE_NEXT
    li GB_F, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_AND_H:
    and GB_A, GB_A, GB_H
    bne GB_A, $zero, DECODE_NEXT
    li GB_F, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_AND_L:
    and GB_A, GB_A, GB_L
    bne GB_A, $zero, DECODE_NEXT
    li GB_F, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_AND_HL:
    jal READ_HL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    and GB_A, GB_A, $v0
    bne GB_A, $zero, DECODE_NEXT
    li GB_F, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
GB_AND_A:
    bne GB_A, $zero, DECODE_NEXT
    li GB_F, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
    nop
GB_XOR_B:
    xor GB_A, GB_A, GB_B
    bne GB_A, $zero, DECODE_NEXT
    li GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_XOR_C:
    xor GB_A, GB_A, GB_C
    bne GB_A, $zero, DECODE_NEXT
    li GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_XOR_D:
    xor GB_A, GB_A, GB_D
    bne GB_A, $zero, DECODE_NEXT
    li GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_XOR_E:
    xor GB_A, GB_A, GB_E
    bne GB_A, $zero, DECODE_NEXT
    li GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_XOR_H:
    xor GB_A, GB_A, GB_H
    bne GB_A, $zero, DECODE_NEXT
    li GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_XOR_L:
    xor GB_A, GB_A, GB_L
    bne GB_A, $zero, DECODE_NEXT
    li GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_XOR_HL:
    jal READ_HL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    xor GB_A, GB_A, $v0
    bne GB_A, $zero, DECODE_NEXT
    li GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
GB_XOR_A:
    addi GB_A, $zero, 0
    j DECODE_NEXT
    li GB_F, Z_FLAG
    nop
    nop
    nop
    nop
    nop
    