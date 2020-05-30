
# 0xBX
GB_OR_B:
    or GB_A, GB_A, GB_B
    bnez GB_A, DECODE_NEXT
    li GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_OR_C:
    or GB_A, GB_A, GB_C
    bnez GB_A, DECODE_NEXT
    li GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_OR_D:
    or GB_A, GB_A, GB_D
    bnez GB_A, DECODE_NEXT
    li GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_OR_E:
    or GB_A, GB_A, GB_E
    bnez GB_A, DECODE_NEXT
    li GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_OR_H:
    or GB_A, GB_A, GB_H
    bnez GB_A, DECODE_NEXT
    li GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_OR_L:
    or GB_A, GB_A, GB_L
    bnez GB_A, DECODE_NEXT
    li GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_OR_HL:
    jal READ_HL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    or GB_A, GB_A, $v0
    bnez GB_A, DECODE_NEXT
    li GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
GB_OR_A:
    bnez GB_A, DECODE_NEXT
    li GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
    nop
GB_CP_B:
    j _CP_A
    addi Param0, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_CP_C:
    j _CP_A
    addi Param0, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_CP_D:
    j _CP_A
    addi Param0, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_CP_E:
    j _CP_A
    addi Param0, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_CP_H:
    j _CP_A
    addi Param0, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_CP_L:
    j _CP_A
    addi Param0, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_CP_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j _CP_A
    addi Param0, $v0, 0
    nop
    nop
GB_CP_A:
    j DECODE_NEXT
    addi GB_F, $zero, Z_FLAG | N_FLAG
_SKIP_JP:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    addi Param0, GB_PC, 2
    jal SET_GB_PC
    andi Param0, Param0, 0xFFFF
    j DECODE_NEXT
    nop
    