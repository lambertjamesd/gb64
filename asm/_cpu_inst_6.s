
### 0x6X
GB_LD_H_B:
    j DECODE_NEXT
    addi GB_H, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_H_C:
    j DECODE_NEXT
    addi GB_H, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_H_D:
    j DECODE_NEXT
    addi GB_H, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_H_E:
    j DECODE_NEXT
    addi GB_H, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_H_H:
    j DECODE_NEXT
    addi GB_H, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_H_L:
    j DECODE_NEXT
    addi GB_H, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_H_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j DECODE_NEXT
    addi GB_H, $v0, 0
    nop
    nop
GB_LD_H_A:
    j DECODE_NEXT
    addi GB_H, GB_A, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_L_B:
    j DECODE_NEXT
    addi GB_L, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_L_C:
    j DECODE_NEXT
    addi GB_L, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_L_D:
    j DECODE_NEXT
    addi GB_L, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_L_E:
    j DECODE_NEXT
    addi GB_L, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_L_H:
    j DECODE_NEXT
    addi GB_L, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_L_L:
    j DECODE_NEXT
    addi GB_L, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_L_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j DECODE_NEXT
    addi GB_L, $v0, 0
    nop
    nop
GB_LD_L_A:
    j DECODE_NEXT
    addi GB_L, GB_A, 0
    nop
    nop
    nop
    nop
    nop
    nop
    