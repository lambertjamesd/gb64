
.data 
    ERROR_MSG: .asciiz "Invalid CPU State\n"
.text

CHECK_FOR_INVALID_STATE:
    slti $at, GB_A, 0x100
    beqz $at, _CHECK_FOR_INVALID_STATE_INVALD
    
    slti $at, GB_F, 0x100
    beqz $at, _CHECK_FOR_INVALID_STATE_INVALD
    
    slti $at, GB_D, 0x100
    beqz $at, _CHECK_FOR_INVALID_STATE_INVALD
    
    slti $at, GB_E, 0x100
    beqz $at, _CHECK_FOR_INVALID_STATE_INVALD
    
    slti $at, GB_H, 0x100
    beqz $at, _CHECK_FOR_INVALID_STATE_INVALD
    
    slti $at, GB_L, 0x100
    beqz $at, _CHECK_FOR_INVALID_STATE_INVALD

    andi $at, GB_PC, 0
    bnez $at, _CHECK_FOR_INVALID_STATE_INVALD
    
    andi $at, GB_SP, 0
    bnez $at, _CHECK_FOR_INVALID_STATE_INVALD

    nop

    jr $ra
    nop
_CHECK_FOR_INVALID_STATE_INVALD:
    la $at, 0x80700000
    sb $zero, 0($at) # useful for memory breakpoint

    save_state_on_stack

    la $a0, ERROR_MSG
    call_c_fn debugInfo
    nop

    restore_state_from_stack

    jr $ra
    nop


.global OPEN_DEBUGGER
.balign 4
OPEN_DEBUGGER:
    save_state_on_stack

    jal CALCULATE_DIV_VALUE
    addi GB_PC, GB_PC, -1 # back PC back to the begginning of current instruction
    andi GB_PC, GB_PC, 0xFFFF
    jal CALCULATE_TIMA_VALUE
    sh GB_PC, CPU_STATE_PC(CPUState)
    
    lw $fp, (ST_FP + _C_CALLBACK_FRAME_SIZE)($sp)

    # $a0 and $a1 already have the correct values
    call_c_fn useDebugger
    nop

    restore_state_from_stack
    
    addi GB_PC, GB_PC, 1 # back PC back to the begginning of current instruction
    andi GB_PC, GB_PC, 0xFFFF

    j DECODE_V0 # the value of the next instruction to execute is returne from useDebugger
    nop
