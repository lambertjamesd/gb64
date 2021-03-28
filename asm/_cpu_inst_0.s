
#### 0x0X
GB_NOP: # start of jump table
    j DECODE_NEXT
    nop
GB_LD_BC_D16:
    jal READ_NEXT_INSTRUCTION # read immedate values
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    jal READ_NEXT_INSTRUCTION
    addi GB_C, $v0, 0 # store C
    j DECODE_NEXT
    addi GB_B, $v0, 0 # store B
GB_LD_BC_A:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    add VAL, GB_A, 0 # write the value to store
    sll ADDR, GB_B, 8 # write upper address
    j GB_DO_WRITE # call store subroutine
    or ADDR, ADDR, GB_C # write lower address
GB_INC_BC:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    addi GB_C, GB_C, 1 # incement the register
    srl $at, GB_C, 8
    andi GB_C, GB_C, 0xFF # keep at 8 bits
    add GB_B, GB_B, $at # add carry bit
    j DECODE_NEXT
    andi GB_B, GB_B, 0xFF # keep at 8 bits
GB_INC_B:
    jal GB_INC # call increment
    addi Param0, GB_B, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_B, Param0, 0 # move register back from call parameter
GB_DEC_B:
    jal GB_DEC # call decrement high bit
    move Param0, GB_B # move register to call parameter
    j DECODE_NEXT
    move GB_B, Param0 # move register back from call parameter
GB_LD_B_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_B, $v0, 0 #store value
GB_RLCA:
    jal GB_RLC_IMPL # do RLC
    addi Param0, GB_A, 0 # store A into param
    addi GB_A, Param0, 0 # store result back into A
    j DECODE_NEXT
    clear_flags Z_FLAG
GB_LD_A16_SP:
    jal READ_NEXT_INSTRUCTION_16
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 4 # update cycles run
    move ADDR, $v0 # use immediate address
    j GB_DO_WRITE_16
    addi VAL, GB_SP, 0 # store value to write
GB_ADD_HL_BC:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll Param0, GB_B, 8 # load high order bits
    j _ADD_TO_HL
    or Param0, Param0, GB_C # load low order bits
GB_LD_A_BC:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_B, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_C # load lower address
    j DECODE_NEXT
    addi GB_A, $v0, 0 # store result into a
GB_DEC_BC:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    addi GB_C, GB_C, -1 # decrement C
    sra $at, GB_C, 8 # shift carry
    add GB_B, GB_B, $at  # add carry to b
    andi GB_C, GB_C, 0xFF # mask C
    j DECODE_NEXT
    andi GB_B, GB_B, 0xFF # mask B
GB_INC_C:
    jal GB_INC # call increment
    addi Param0, GB_C, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_C, Param0, 0 # move register back from call parameter
GB_DEC_C:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_C, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_C, Param0, 0 # move register back from call parameter
GB_LD_C_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_C, $v0, 0 #store value
GB_RRCA:
    jal GB_RRC_IMPL # call rotate
    addi Param0, GB_A, 0 # move register to call parameter
    addi GB_A, Param0, 0 # store paramter back
    j DECODE_NEXT
    clear_flags Z_FLAG
    