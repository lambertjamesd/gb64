
#### 0x1X
GB_STOP:
    # STOP will skip the next instruction
    addi GB_PC, GB_PC, 1
    jal CHECK_FOR_SPEED_SWITCH
    addi PC_MEM_POINTER, PC_MEM_POINTER, 1
    bnez $v0, DECODE_NEXT
    addi $at, $zero, STOP_REASON_STOP
    j GB_SIMULATE_HALTED # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
GB_LD_DE_D16:
    lbu GB_E, 0(PC_MEM_POINTER)
    lbu GB_D, 1(PC_MEM_POINTER)
    addi PC_MEM_POINTER, PC_MEM_POINTER, 2
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    j DECODE_NEXT
    addi GB_PC, GB_PC, 2
GB_LD_DE_A:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    add VAL, GB_A, 0 # write the value to store
    sll ADDR, GB_D, 8 # write upper address
    j GB_DO_WRITE # call store subroutine
    or ADDR, ADDR, GB_E # write lower address
GB_INC_DE:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    addi GB_E, GB_E, 1 # incement the register
    srl $at, GB_E, 8
    andi GB_E, GB_E, 0xFF # keep at 8 bits
    add GB_D, GB_D, $at # add carry bit
    j DECODE_NEXT
    andi GB_D, GB_D, 0xFF # keep at 8 bits
GB_INC_D:
    jal GB_INC # call increment
    addi Param0, GB_D, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_D, Param0, 0 # move register back from call parameter
GB_DEC_D:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_D, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_D, Param0, 0 # move register back from call parameter
GB_LD_D_D8:
    lbu GB_D, 0(PC_MEM_POINTER)
    addi PC_MEM_POINTER, PC_MEM_POINTER, 1
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_PC, GB_PC, 1
GB_RLA:
    jal GB_RL_IMPL # do RLC
    addi Param0, GB_A, 0 # store A into param
    addi GB_A, Param0, 0 # store result back into A
    j DECODE_NEXT
    clear_flags Z_FLAG
GB_JR:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    sll $v0, $v0, 24 # sign extend the bytes
    sra $v0, $v0, 24
    add Param0, GB_PC, $v0
    jal SET_GB_PC
    andi Param0, Param0, 0xFFFF
    j DECODE_NEXT
GB_ADD_HL_DE:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll Param0, GB_D, 8 # load high order bits
    j _ADD_TO_HL
    or Param0, Param0, GB_E # load low order bits
GB_LD_A_DE:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_D, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_E # load lower address
    j DECODE_NEXT
    addi GB_A, $v0, 0 # store result into a
GB_DEC_DE:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    addi GB_E, GB_E, -1 # decrement E
    sra $at, GB_E, 8 # shift carry
    add GB_D, GB_D, $at  # add carry to D
    andi GB_E, GB_E, 0xFF # mask E
    j DECODE_NEXT
    andi GB_D, GB_D, 0xFF # mask D
GB_INC_E:
    jal GB_INC # call increment
    addi Param0, GB_E, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_E, Param0, 0 # move register back from call parameter
GB_DEC_E:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_E, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_E, Param0, 0 # move register back from call parameter
GB_LD_E_D8:
    lbu GB_E, 0(PC_MEM_POINTER)
    addi PC_MEM_POINTER, PC_MEM_POINTER, 1
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_PC, GB_PC, 1
GB_RRA:
    jal GB_RR_IMPL
    move Param0, GB_A
    move GB_A, Param0
    j DECODE_NEXT
    clear_flags Z_FLAG | N_FLAG | H_FLAG
    