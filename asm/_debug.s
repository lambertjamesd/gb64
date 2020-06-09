
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
