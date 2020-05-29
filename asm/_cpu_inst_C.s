
### 0xCX
GB_RET_NZ:
    andi $at, GB_F, Z_FLAG
    bne $at, $zero, DECODE_NEXT # if Z_FLAG != 0 skip return
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _GB_RET
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    nop
    nop
    nop
GB_POP_BC:
    addi ADDR, GB_SP, 0
    jal GB_DO_READ_16
    addi GB_SP, GB_SP, 2
    andi GB_SP, GB_SP, 0xFFFF
    srl GB_B, $v0, 8 # store B
    andi GB_C, $v0, 0xFF # store C
    j DECODE_NEXT
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
GB_JP_NZ:
    andi $at, GB_F, Z_FLAG
    bne $at, $zero, _SKIP_JP # if Z_FLAG != 0 skip jump
    nop
    jal GB_JP
    nop
    nop
    nop
    nop
GB_JP:
    jal READ_NEXT_INSTRUCTION_16
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    jal SET_GB_PC
    move Param0, $v0
    j DECODE_NEXT
    nop
    nop
    nop
GB_CALL_NZ:
    andi $at, GB_F, Z_FLAG
    bne $at, $zero, _SKIP_JP # if Z_FLAG != 0 skip the call
    nop
    j _GB_CALL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 4 # update cycles run
    nop
    nop
    nop
GB_PUSH_BC:
    addi GB_SP, GB_SP, -2
    andi GB_SP, GB_SP, 0xFFFF
    addi ADDR, GB_SP, 0
    sll VAL, GB_B, 8
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 3 # update cycles run
    j GB_DO_WRITE_16
    or VAL, VAL, GB_C
    nop
GB_ADD_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _ADD_TO_A
    addi Param0, $v0, 0
    nop
    nop
    nop
    nop
GB_RST_00H:
    addi GB_SP, GB_SP, -2
    andi ADDR, GB_SP, 0xFFFF
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    jal SET_GB_PC
    addi Param0, $zero, 0x0000
    j GB_DO_WRITE_16
    move GB_SP, ADDR
GB_RET_Z:
    andi $at, GB_F, Z_FLAG
    beq $at, $zero, DECODE_NEXT # if Z_FLAG == 0 skip RET
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _GB_RET
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    nop
    nop
    nop
GB_RET:
    j _GB_RET
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    nop
    nop
    nop
    nop
    nop
    nop
GB_JP_Z:
    andi $at, GB_F, Z_FLAG
    beq $at, $zero, _SKIP_JP # if Z_FLAG == 0 skip jump
    nop
    jal GB_JP
    nop
    nop
    nop
    nop
GB_PREFIX_CB:
    j _GB_PREFIX_CB
    nop
    nop
    nop
    nop
    nop
    nop
    nop
GB_CALL_Z:
    andi $at, GB_F, Z_FLAG
    beq $at, $zero, _SKIP_JP # if Z_FLAG == 0 skip call
    nop
    j _GB_CALL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 4 # update cycles run
    nop
    nop
    nop
GB_CALL:
    j _GB_CALL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 4 # update cycles run
    nop
    nop
    nop
    nop
    nop
    nop
GB_ADC_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _ADC_TO_A
    addi Param0, $v0, 0
    nop
    nop
    nop
    nop
GB_RST_08H:
    addi GB_SP, GB_SP, -2
    andi ADDR, GB_SP, 0xFFFF
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    jal SET_GB_PC
    addi Param0, $zero, 0x0008
    j GB_DO_WRITE_16
    move GB_SP, ADDR
    