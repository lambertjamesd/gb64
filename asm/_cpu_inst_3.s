
### 0x3X
GB_JR_NC:
    andi $at, GB_F, C_FLAG # check z flag
    bnez $at, _SKIP_JR # skip jump if not zero
    nop
    j GB_JR
    nop
GB_LD_SP_D16:
    jal READ_NEXT_INSTRUCTION_16 # read immedate values
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    j DECODE_NEXT
    move GB_SP, $v0
GB_LDD_HL_A:
    add VAL, GB_A, 0 # write the value to store
    sll ADDR, GB_H, 8 # write upper address
    or ADDR, ADDR, GB_L # write lower address

    addi GB_L, ADDR, -1 # decrement address
    srl GB_H, GB_L, 8 # store upper bits
    andi GB_L, GB_L, 0xFF # mask lower bits
    andi GB_H, GB_H, 0xFF # mask upper bits bits
    j GB_DO_WRITE # call store subroutine
    # intentially leave off nop to overflow to next instruction
GB_INC_SP:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    addi GB_SP, GB_SP, 1
    j DECODE_NEXT
    andi GB_SP, GB_SP, 0xFFFF
GB_INC_HL_ADDR:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    sll ADDR, GB_H, 8 # write upper address
    jal GB_DO_READ
    or ADDR, ADDR, GB_L # write lower address
    jal GB_INC # call increment
    move Param0, $v0 # move loaded value to call parameter
    j GB_DO_WRITE
    move VAL, Param0
GB_DEC_HL_ADDR:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    sll ADDR, GB_H, 8 # write upper address
    jal GB_DO_READ
    or ADDR, ADDR, GB_L # write lower address
    jal GB_DEC # call decrement
    move Param0, $v0 # move loaded value to call parameter
    j GB_DO_WRITE
    move VAL, Param0
GB_LD_HL_ADDR_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    add VAL, $v0, 0 # write the value to store
    sll ADDR, GB_H, 8 # write upper address
    j GB_DO_WRITE
    or ADDR, ADDR, GB_L # write lower address
GB_SCF:
    clear_flags N_FLAG | H_FLAG
    j DECODE_NEXT
    set_flags C_FLAG
GB_JR_C:
    andi $at, GB_F, C_FLAG # check z flag
    beqz $at, _SKIP_JR # skip jump if not zero
    nop
    j GB_JR
    nop
GB_ADD_HL_SP:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _ADD_TO_HL
    move Param0, GB_SP
GB_LDD_A_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    addi GB_L, ADDR, -1 # decrement L
    jal GB_DO_READ # call read instruction
    srl GB_H, GB_L, 8 # store incremented H
    addi GB_A, $v0, 0 # store result into a
    andi GB_L, GB_L, 0xFF # mask lower bits
    j DECODE_NEXT
    andi GB_H, GB_H, 0xFF # mask upper bits bits
GB_DEC_SP:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    addi GB_SP, GB_SP, -1
    j DECODE_NEXT
    andi GB_SP, GB_SP, 0xFFFF
GB_INC_A:
    jal GB_INC # call increment
    addi Param0, GB_A, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_A, Param0, 0 # move register back from call parameter
GB_DEC_A:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_A, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_A, Param0, 0 # move register back from call parameter
GB_LD_A_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_A, $v0, 0 #store value
GB_CCF:
    clear_flags N_FLAG | H_FLAG
    j DECODE_NEXT
    xori GB_F, GB_F, C_FLAG
    