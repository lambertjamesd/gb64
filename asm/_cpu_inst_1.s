
#### 0x1X
GB_STOP:
    jal READ_NEXT_INSTRUCTION # STOP will skip the next instruction
    nop
    addi $at, $zero, STOP_REASON_STOP
    j GB_SIMULATE_HALTED # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
GB_LD_DE_D16:
    jal READ_NEXT_INSTRUCTION # read immedate values
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    jal READ_NEXT_INSTRUCTION
    addi GB_E, $v0, 0 # store E
    j DECODE_NEXT
    addi GB_D, $v0, 0 # store D
    nop
    nop
GB_LD_DE_A:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    add VAL, GB_A, 0 # write the value to store
    sll ADDR, GB_D, 8 # write upper address
    j GB_DO_WRITE # call store subroutine
    or ADDR, ADDR, GB_E # write lower address
    nop
    nop
    nop
GB_INC_DE:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    addi GB_E, GB_E, 1 # incement the register
    srl $at, GB_E, 8
    andi GB_E, GB_E, 0xFF # keep at 8 bits
    add GB_D, GB_D, $at # add carry bit
    j DECODE_NEXT
    andi GB_D, GB_D, 0xFF # keep at 8 bits
    nop
GB_INC_D:
    jal GB_INC # call increment
    addi Param0, GB_D, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_D, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_DEC_D:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_D, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_D, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_LD_D_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_D, $v0, 0 #store value
    nop
    nop
    nop
    nop
GB_RLA:
    jal GB_RL_IMPL # do RLC
    addi Param0, GB_A, 0 # store A into param
    addi GB_A, Param0, 0 # store result back into A
    j DECODE_NEXT
    clear_flags Z_FLAG
    nop
    nop
    nop
GB_JR:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR  * 2 # update cycles run
    sll $v0, $v0, 24 # sign extend the bytes
    sra $v0, $v0, 24
    jal SET_GB_PC
    add Param0, GB_PC, $v0
    j DECODE_NEXT
    nop
GB_ADD_HL_DE:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll Param0, GB_D, 8 # load high order bits
    j _ADD_TO_HL
    or Param0, Param0, GB_E # load low order bits
    nop
    nop
    nop
    nop
GB_LD_A_DE:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_D, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_E # load lower address
    j DECODE_NEXT
    addi GB_A, $v0, 0 # store result into a
    nop
    nop
GB_DEC_DE:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    addi GB_E, GB_E, -1 # decrement E
    sra $at, GB_E, 8 # shift carry
    add GB_D, GB_D, $at  # add carry to D
    andi GB_E, GB_E, 0xFF # mask E
    j DECODE_NEXT
    andi GB_D, GB_D, 0xFF # mask D
    nop
GB_INC_E:
    jal GB_INC # call increment
    addi Param0, GB_E, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_E, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_DEC_E:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_E, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_E, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_LD_E_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_E, $v0, 0 #store value
    nop
    nop
    nop
    nop
GB_RRA:
    jal GB_RR_IMPL
    move Param0, GB_A
    move GB_A, Param0
    j DECODE_NEXT
    clear_flags Z_FLAG | N_FLAG | H_FLAG
    nop
    nop
    nop
    