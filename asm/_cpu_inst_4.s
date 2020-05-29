
### 0x4X
GB_LD_B_B:
    j DECODE_NEXT
    nop
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_B_C:
    j DECODE_NEXT
    addi GB_B, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_B_D:
    j DECODE_NEXT
    addi GB_B, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_B_E:
    j DECODE_NEXT
    addi GB_B, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_B_H:
    j DECODE_NEXT
    addi GB_B, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_B_L:
    j DECODE_NEXT
    addi GB_B, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_B_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j DECODE_NEXT
    addi GB_B, $v0, 0
    nop
    nop
GB_LD_B_A:
    j DECODE_NEXT
    addi GB_B, GB_A, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_C_B:
    j DECODE_NEXT
    addi GB_C, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_C_C:
    j DECODE_NEXT
    nop
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_C_D:
    j DECODE_NEXT
    addi GB_C, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_C_E:
    j DECODE_NEXT
    addi GB_C, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_C_H:
    j DECODE_NEXT
    addi GB_C, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_C_L:
    j DECODE_NEXT
    addi GB_C, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_C_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j DECODE_NEXT
    addi GB_C, $v0, 0
    nop
    nop
GB_LD_C_A:
    j DECODE_NEXT
    addi GB_C, GB_A, 0
    nop
    nop
    nop
    nop
    nop
    nop
    