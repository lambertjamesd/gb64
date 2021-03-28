

#######################
# Pop top of stack and return to it
#######################

_GB_RET:
    addi ADDR, GB_SP, 0
    jal GB_DO_READ_16
    addi GB_SP, GB_SP, 2
    andi GB_SP, GB_SP, 0xFFFF
    jal SET_GB_PC
    move Param0, $v0
    j DECODE_NEXT
    nop
    
_GB_CALL:
    addi GB_SP, GB_SP, -2
    andi GB_SP, GB_SP, 0xFFFF
    addi VAL, GB_PC, 2

    lbu $at, 0(PC_MEM_POINTER)
    lbu Param0, 1(PC_MEM_POINTER)
    sll Param0, Param0, 8
    jal SET_GB_PC
    or Param0, Param0, $at
    j GB_DO_WRITE_16
    addi ADDR, GB_SP, 0
        
#######################
# Subtract Param0 to A
#######################
    
_SKIP_JR:
    jal SET_GB_PC
    addi Param0, GB_PC, 1
    j DECODE_NEXT
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    