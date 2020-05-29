
### 0x9X
GB_SUB_B:
    j _SUB_FROM_A
    addi Param0, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SUB_C:
    j _SUB_FROM_A
    addi Param0, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SUB_D:
    j _SUB_FROM_A
    addi Param0, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SUB_E:
    j _SUB_FROM_A
    addi Param0, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SUB_H:
    j _SUB_FROM_A
    addi Param0, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SUB_L:
    j _SUB_FROM_A
    addi Param0, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SUB_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j _SUB_FROM_A
    addi Param0, $v0, 0
    nop
    nop
GB_SUB_A:
    addi GB_A, $zero, 0
    j DECODE_NEXT
    addi GB_F, $zero, Z_FLAG | N_FLAG
    nop
    nop
    nop
    nop
    nop
GB_SBC_B:
    j _SBC_FROM_A
    addi Param0, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SBC_C:
    j _SBC_FROM_A
    addi Param0, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SBC_D:
    j _SBC_FROM_A
    addi Param0, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SBC_E:
    j _SBC_FROM_A
    addi Param0, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SBC_H:
    j _SBC_FROM_A
    addi Param0, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SBC_L:
    j _SBC_FROM_A
    addi Param0, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SBC_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j _SBC_FROM_A
    addi Param0, $v0, 0
    nop
    nop
GB_SBC_A:
    j _SBC_FROM_A
    addi Param0, GB_A, 0
    nop
    nop
    nop
    nop
    nop
    nop
    