
### 0xFX
GB_LDH_A_a8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    jal GB_DO_READ_REGISTERS
    ori ADDR, $v0, 0xFF00
    j DECODE_NEXT
    move GB_A, $v0
    nop
    nop
GB_POP_AF:
    addi ADDR, GB_SP, 0
    jal GB_DO_READ_16
    addi GB_SP, GB_SP, 2
    andi GB_SP, GB_SP, 0xFFFF
    srl GB_A, $v0, 8 # store A
    andi GB_F, $v0, 0xF0 # store F
    j DECODE_NEXT
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
GB_LDH_A_C:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    jal GB_DO_READ_REGISTERS
    ori ADDR, GB_C, 0xFF00
    j DECODE_NEXT
    move GB_A, $v0
    nop
    nop
    nop
GB_DI:
    j DECODE_NEXT
    sb $zero, CPU_STATE_INTERRUPTS(CPUState)
    nop
    nop
    nop
    nop
    nop
    nop
GB_ERROR_8:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_PUSH_AF:
    addi GB_SP, GB_SP, -2
    andi GB_SP, GB_SP, 0xFFFF
    addi ADDR, GB_SP, 0
    sll VAL, GB_A, 8
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 3 # update cycles run
    j GB_DO_WRITE_16
    or VAL, VAL, GB_F
    nop
GB_OR_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    or GB_A, GB_A, $v0
    bnez GB_A, DECODE_NEXT
    li GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
GB_RST_30H:
    addi GB_SP, GB_SP, -2
    andi ADDR, GB_SP, 0xFFFF
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    jal SET_GB_PC
    addi Param0, $zero, 0x0030
    j GB_DO_WRITE_16
    move GB_SP, ADDR
GB_LD_HL_SP_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    sll $v0, $v0, 24 #sign extend
    j _ADD_TO_HL_SP
    sra Param0, $v0, 24
    nop
    nop
    nop
GB_LD_SP_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll GB_SP, GB_H, 8
    j DECODE_NEXT
    or GB_SP, GB_SP, GB_L
    nop
    nop
    nop
    nop
GB_LD_A_a16:
    jal READ_NEXT_INSTRUCTION_16
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 3 # update cycles run
    jal GB_DO_READ
    move ADDR, $v0
    move GB_A, $v0
    j DECODE_NEXT
    nop
    nop
GB_EI:
    li $at, INTERRUPTS_ENABLED
    jal CHECK_FOR_INTERRUPT
    sb $at, CPU_STATE_INTERRUPTS(CPUState)
    j DECODE_NEXT
    nop
    nop
    nop
    nop
GB_ERROR_9:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_ERROR_10:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_CP_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _CP_A
    addi Param0, $v0, 0
    nop
    nop
    nop
    nop
GB_RST_38H:
    addi GB_SP, GB_SP, -2
    andi ADDR, GB_SP, 0xFFFF
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    jal SET_GB_PC
    addi Param0, $zero, 0x0038
    j GB_DO_WRITE_16
    move GB_SP, ADDR
    