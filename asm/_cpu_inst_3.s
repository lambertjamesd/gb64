
### 0x3X
GB_JR_NC:
    andi $at, GB_F, C_FLAG # check z flag
    bne $at, $zero, _SKIP_JR # skip jump if not zero
    nop
    j GB_JR
    nop
    nop
    nop
    nop
GB_LD_SP_D16:
    jal READ_NEXT_INSTRUCTION_16 # read immedate values
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    j DECODE_NEXT
    move GB_SP, $v0
    nop
    nop
    nop
    nop
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
    nop
    nop
    nop
    nop
GB_INC_HL_ADDR:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    sll ADDR, GB_H, 8 # write upper address
    jal GB_DO_READ
    or ADDR, ADDR, GB_L # write lower address
    jal GB_INC # call increment
    addi Param0, $v0, 0 # move loaded value to call parameter
    j DECODE_NEXT
    sb Param0, 0(ADDR) # use same ADDR calculated in GB_DO_READ
GB_DEC_HL_ADDR:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    sll ADDR, GB_H, 8 # write upper address
    jal GB_DO_READ
    or ADDR, ADDR, GB_L # write lower address
    jal GB_DEC # call decrement
    addi Param0, $v0, 0 # move loaded value to call parameter
    j DECODE_NEXT
    sb Param0, 0(ADDR) # use same ADDR calculated in GB_DO_READ
GB_LD_HL_ADDR_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    add VAL, $v0, 0 # write the value to store
    sll ADDR, GB_H, 8 # write upper address
    j GB_DO_WRITE
    or ADDR, ADDR, GB_L # write lower address
    nop
    nop
GB_SCF:
    clear_flags N_FLAG | H_FLAG
    j DECODE_NEXT
    set_flags C_FLAG
    nop
    nop
    nop
    nop
    nop
GB_JR_C:
    andi $at, GB_F, C_FLAG # check z flag
    beq $at, $zero, _SKIP_JR # skip jump if not zero
    nop
    j GB_JR
    nop
    nop
    nop
    nop
GB_ADD_HL_SP:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _ADD_TO_HL
    move Param0, GB_SP
    nop
    nop
    nop
    nop
    nop
GB_LDD_A_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    jal GB_DO_READ # call read instruction
    addi GB_L, ADDR, -1 # decrement L
    addi GB_A, $v0, 0 # store result into a
    j _MASK_HL
    srl GB_H, GB_L, 8 # store incremented H
GB_DEC_SP:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    addi GB_SP, GB_SP, -1
    j DECODE_NEXT
    andi GB_SP, GB_SP, 0xFFFF
    nop
    nop
    nop
    nop
GB_INC_A:
    jal GB_INC # call increment
    addi Param0, GB_A, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_A, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_DEC_A:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_A, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_A, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_LD_A_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_A, $v0, 0 #store value
    nop
    nop
    nop
    nop
GB_CCF:
    clear_flags N_FLAG | H_FLAG
    j DECODE_NEXT
    xori GB_F, GB_F, C_FLAG
    nop
    nop
    nop
    nop
    nop
    