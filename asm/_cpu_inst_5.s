
### 0x5X
GB_LD_D_B:
    j DECODE_NEXT
    addi GB_D, GB_B, 0
GB_LD_D_C:
    j DECODE_NEXT
    addi GB_D, GB_C, 0
GB_LD_D_D:
    j DECODE_NEXT
    addi GB_D, GB_D, 0
GB_LD_D_E:
    j DECODE_NEXT
    addi GB_D, GB_E, 0
GB_LD_D_H:
    j DECODE_NEXT
    addi GB_D, GB_H, 0
GB_LD_D_L:
    j DECODE_NEXT
    addi GB_D, GB_L, 0
GB_LD_D_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j DECODE_NEXT
    addi GB_D, $v0, 0
GB_LD_D_A:
    j DECODE_NEXT
    addi GB_D, GB_A, 0
GB_LD_E_B:
    j DECODE_NEXT
    addi GB_E, GB_B, 0
GB_LD_E_C:
    j DECODE_NEXT
    addi GB_E, GB_C, 0
GB_LD_E_D:
    j DECODE_NEXT
    addi GB_E, GB_D, 0
GB_LD_E_E:
    j DECODE_NEXT
    addi GB_E, GB_E, 0
GB_LD_E_H:
    j DECODE_NEXT
    addi GB_E, GB_H, 0
GB_LD_E_L:
    j DECODE_NEXT
    addi GB_E, GB_L, 0
GB_LD_E_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j DECODE_NEXT
    addi GB_E, $v0, 0
GB_LD_E_A:
    j DECODE_NEXT
    addi GB_E, GB_A, 0
    