
### 0xEX
GB_LDH_a8_A:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    ori ADDR, $v0, 0xFF00
    j GB_DO_WRITE_REGISTERS
    move VAL, GB_A
    nop
    nop
    nop
GB_POP_HL:
    addi ADDR, GB_SP, 0
    jal GB_DO_READ_16
    addi GB_SP, GB_SP, 2
    andi GB_SP, GB_SP, 0xFFFF
    srl GB_H, $v0, 8 # store B
    andi GB_L, $v0, 0xFF # store C
    j DECODE_NEXT
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
GB_LDH_C_A:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    addi ADDR, GB_C, 0
    ori ADDR, ADDR, 0xFF00
    j GB_DO_WRITE_REGISTERS
    addi VAL, GB_A, 0
    nop
    nop
    nop
GB_ERROR_3:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_ERROR_4:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_PUSH_HL:
    addi GB_SP, GB_SP, -2
    andi GB_SP, GB_SP, 0xFFFF
    addi ADDR, GB_SP, 0
    sll VAL, GB_H, 8
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 3 # update cycles run
    j GB_DO_WRITE_16
    or VAL, VAL, GB_L
    nop
GB_AND_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    and GB_A, GB_A, $v0
    bne GB_A, $zero, DECODE_NEXT
    ori GB_F, $zero, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
GB_RST_20H:
    addi GB_SP, GB_SP, -2
    andi ADDR, GB_SP, 0xFFFF
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 3 # update cycles run
    addi VAL, GB_PC, 0
    jal SET_GB_PC
    addi Param0, $zero, 0x0020
    j GB_DO_WRITE_16
    move GB_SP, ADDR
GB_ADD_SP_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 3 # update cycles run
    sll $v0, $v0, 24 #sign extend
    j _ADD_TO_SP
    sra Param0, $v0, 24
    nop
    nop
    nop
GB_JP_HL:
    sll Param0, GB_H, 8
    jal SET_GB_PC
    or Param0, Param0, GB_L
    j DECODE_NEXT
    nop
    nop
    nop
    nop
GB_LD_a16_A:
    jal READ_NEXT_INSTRUCTION_16
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 3 # update cycles run
    addi ADDR, $v0, 0
    j GB_DO_WRITE
    addi VAL, GB_A, 0
    nop
    nop
    nop
GB_ERROR_5:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_ERROR_6:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_ERROR_7:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_XOR_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    xor GB_A, GB_A, $v0
    bne GB_A, $zero, DECODE_NEXT
    andi GB_F, $zero, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
GB_RST_28H:
    addi GB_SP, GB_SP, -2
    andi ADDR, GB_SP, 0xFFFF
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 3 # update cycles run
    addi VAL, GB_PC, 0
    jal SET_GB_PC
    addi Param0, $zero, 0x0028
    j GB_DO_WRITE_16
    move GB_SP, ADDR
    