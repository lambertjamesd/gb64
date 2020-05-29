

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
    jal READ_NEXT_INSTRUCTION_16
    addi GB_SP, GB_SP, -2
    andi GB_SP, GB_SP, 0xFFFF
    move VAL, GB_PC
    jal SET_GB_PC
    move Param0, $v0
    j GB_DO_WRITE_16
    addi ADDR, GB_SP, 0
        
#######################
# Subtract Param0 to A
#######################
    
_SKIP_JR:
    addi Param0, GB_PC, 1
    jal SET_GB_PC
    andi Param0, Param0, 0xFFFF
    j DECODE_NEXT
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    