
### 0XDX
GB_RET_NC:
    andi $at, GB_F, C_FLAG
    bne $at, $zero, DECODE_NEXT # if C_FLAG != 0 skip return
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _GB_RET
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    nop
    nop
    nop
GB_POP_DE:
    addi ADDR, GB_SP, 0
    jal GB_DO_READ_16
    addi GB_SP, GB_SP, 2
    andi GB_SP, GB_SP, 0xFFFF
    srl GB_D, $v0, 8 # store B
    andi GB_E, $v0, 0xFF # store C
    j DECODE_NEXT
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
GB_JP_NC:
    andi $at, GB_F, C_FLAG
    bne $at, $zero, _SKIP_JP # if Z_FLAG != 0 skip jump
    nop
    jal GB_JP
    nop
    nop
    nop
    nop
GB_ERROR_0:
    j OPEN_DEBUGGER # exit early
    nop
    nop
    nop
    nop
    nop
    nop
    nop
GB_CALL_NC:
    andi $at, GB_F, C_FLAG
    bne $at, $zero, _SKIP_JP # if Z_FLAG != 0 skip the call
    nop
    j _GB_CALL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 5 # update cycles run
    nop
    nop
    nop
GB_PUSH_DE:
    addi GB_SP, GB_SP, -2
    andi GB_SP, GB_SP, 0xFFFF
    addi ADDR, GB_SP, 0
    sll VAL, GB_D, 8
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 3 # update cycles run
    j GB_DO_WRITE_16
    or VAL, VAL, GB_E
    nop
GB_SUB_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _SUB_FROM_A
    addi Param0, $v0, 0
    nop
    nop
    nop
    nop
GB_RST_10H:
    addi GB_SP, GB_SP, -2
    andi ADDR, GB_SP, 0xFFFF
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    jal SET_GB_PC
    addi Param0, $zero, 0x0010
    j GB_DO_WRITE_16
    move GB_SP, ADDR
GB_RET_C:
    andi $at, GB_F, C_FLAG
    beq $at, $zero, DECODE_NEXT # if Z_FLAG == 0 skip RET
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _GB_RET
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    nop
    nop
    nop
GB_RETI:
    li $at, INTERRUPTS_ENABLED
    jal CHECK_FOR_INTERRUPT
    sb $at, CPU_STATE_INTERRUPTS(CPUState)
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    j _GB_RET
    nop
    nop
    nop
GB_JP_C:
    andi $at, GB_F, C_FLAG
    beq $at, $zero, _SKIP_JP # if Z_FLAG == 0 skip jump
    nop
    jal GB_JP
    nop
    nop
    nop
    nop
GB_ERROR_1:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_CALL_C:
    andi $at, GB_F, C_FLAG
    beq $at, $zero, _SKIP_JP # if Z_FLAG == 0 skip call
    nop
    j _GB_CALL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 5 # update cycles run
    nop
    nop
    nop
GB_ERROR_2:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_SBC_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _SBC_FROM_A
    addi Param0, $v0, 0
    nop
    nop
    nop
    nop
GB_RST_18H:
    addi GB_SP, GB_SP, -2
    andi ADDR, GB_SP, 0xFFFF
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    jal SET_GB_PC
    addi Param0, $zero, 0x0018
    j GB_DO_WRITE_16
    move GB_SP, ADDR
    