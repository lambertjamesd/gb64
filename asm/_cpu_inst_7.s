
### 0x7X
GB_LD_HL_B:    
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_B, 0
    nop
    nop
    nop
GB_LD_HL_C:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_C, 0
    nop
    nop
    nop
GB_LD_HL_D:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_D, 0
    nop
    nop
    nop
GB_LD_HL_E:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_E, 0
    nop
    nop
    nop
GB_LD_HL_H:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_H, 0
    nop
    nop
    nop
GB_LD_HL_L:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_L, 0
    nop
    nop
    nop
GB_HALT:
    li $at, STOP_REASON_HALT
    jal CHECK_FOR_UNHALT
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    lbu $at, CPU_STATE_STOP_REASON(CPUState)
    beqz $at, DECODE_NEXT
    nop
    j GB_SIMULATE_HALTED
    nop
GB_LD_HL_A:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_A, 0
    nop
    nop
    nop
GB_LD_A_B:
    j DECODE_NEXT
    addi GB_A, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_A_C:
    j DECODE_NEXT
    addi GB_A, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_A_D:
    j DECODE_NEXT
    addi GB_A, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_A_E:
    j DECODE_NEXT
    addi GB_A, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_A_H:
    j DECODE_NEXT
    addi GB_A, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_A_L:
    j DECODE_NEXT
    addi GB_A, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_A_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j DECODE_NEXT
    addi GB_A, $v0, 0
    nop
    nop
GB_LD_A_A:
    j DECODE_NEXT
    addi GB_A, GB_A, 0
    nop
    nop
    nop
    nop
    nop
    nop
    