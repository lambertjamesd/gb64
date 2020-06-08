
.data 
    ERROR_MSG: .asciiz "Invalid CPU State %c\n"
.text

CHECK_FOR_INVALID_STATE:
    slti $at, GB_A, 0x100
    beqz $at, _CHECK_FOR_INVALID_STATE_INVALD
    li TMP2, 'A'
    
    slti $at, GB_F, 0x100
    beqz $at, _CHECK_FOR_INVALID_STATE_INVALD
    li TMP2, 'F'
    
    slti $at, GB_D, 0x100
    beqz $at, _CHECK_FOR_INVALID_STATE_INVALD
    li TMP2, 'D'
    
    slti $at, GB_E, 0x100
    beqz $at, _CHECK_FOR_INVALID_STATE_INVALD
    li TMP2, 'E'
    
    slti $at, GB_H, 0x100
    beqz $at, _CHECK_FOR_INVALID_STATE_INVALD
    li TMP2, 'H'
    
    slti $at, GB_L, 0x100
    beqz $at, _CHECK_FOR_INVALID_STATE_INVALD
    li TMP2, 'L'

    andi $at, GB_PC, 0
    bnez $at, _CHECK_FOR_INVALID_STATE_INVALD
    li TMP2, 'P'
    
    andi $at, GB_SP, 0
    bnez $at, _CHECK_FOR_INVALID_STATE_INVALD
    li TMP2, 'S'

    jr $ra
    nop
_CHECK_FOR_INVALID_STATE_INVALD:
    la $at, 0x80700000
    sb $zero, 0($at) # useful for memory breakpoint

    addi $sp, $sp, -80
    move $at, $sp
    save_state_on_stack

    la $a0, ERROR_MSG
    move $a1, $at
    call_c_fn sprintf 
    move $a2, TMP2

    call_c_fn debugInfo
    move $a0, $a1

    restore_state_from_stack

    jr $ra
    addi $sp, $sp, 80
