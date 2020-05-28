

#######################
# Increments the high byte of Param0
# and sets any associated flags
#######################
GB_INC:
    addi Param0, Param0, 0x1
    andi Param0, Param0, 0xFF # keep register at 8 bits
    bne Param0, $zero, _GB_INC_CHECK_HALF
    clear_flags Z_FLAG | N_FLAG
    jr $ra
    set_flags Z_FLAG | H_FLAG # set ZH flags

_GB_INC_CHECK_HALF:
    andi $at, Param0, 0x3 # check if h flag should be set
    bne $at, $zero, _GB_INC_DONE
    nop
    jr $ra
    set_flags H_FLAG # set H flag

_GB_INC_DONE:
    jr $ra
    clear_flags H_FLAG # clear H flag

#######################
# Decrements the high byte of Param0
# and sets any associated flags
#######################
GB_DEC:
    addi Param0, Param0, 0xFF
    andi Param0, Param0, 0xFF # keep register at 8 bits
    bne Param0, $zero, _GB_DEC_CHECK_HALF
    set_flags N_FLAG # set N flag

    clear_flags H_FLAG # clear H flag
    jr $ra
    set_flags Z_FLAG # set Z flag

_GB_DEC_CHECK_HALF:
    andi $at, Param0, 0x3 # check if h flag should be set
    addi $at, $at, -0x3
    bne $at, $zero, _GB_DEC_DONE
    clear_flags Z_FLAG
    jr $ra
    clear_flags H_FLAG # clear H flag

_GB_DEC_DONE:
    jr $ra
    set_flags H_FLAG # set H flag

#######################
# Rotates Param0 1 bit left and 
# sets flags
#######################

GB_RLC_IMPL:
    beq Param0, $zero, _GB_BITWISE_IMPL_IS_ZERO
    sll Param0, Param0, 1 # shift the bit once
    srl $at, Param0, 8 # shift carry bit back
    or Param0, Param0, $at # put rotated bit back
    andi Param0, Param0, 0xFF # keep param 8 bits
    sll GB_F, $at, 4 # shift carry bit into C_FLAG position
    jr $ra
    andi GB_F, GB_F, C_FLAG # mask c flag
_GB_BITWISE_IMPL_IS_ZERO:
    jr $ra
    ori GB_F, $zero, Z_FLAG # set Z_FLAG

#######################
# Rotates Param0 1 bit left and 
# sets flags
#######################

GB_RRC_IMPL:
    beq Param0, $zero, _GB_BITWISE_IMPL_IS_ZERO
    sll $at, Param0, 7 # shift carry bit back
    srl Param0, Param0, 1 # shift bit once
    or Param0, Param0, $at # put rotated bit back
    andi Param0, Param0, 0xFF # keep param 8 bits
    andi $at, $at, 0x80 # mask carry bit
    srl GB_F, $at, 3 # move carry bit into C_FLAG
    jr $ra
    andi GB_F, GB_F, C_FLAG # mask c flag
    
#######################
# Rotates Param0 1 bit left through carry and 
# sets flags
#######################

GB_RL_IMPL:
    sll Param0, Param0, 1 # shift the bit once
    srl $at, GB_F, 4 # shift carry bit into position
    andi $at, $at, 0x1 # mask carry bit
    or Param0, Param0, $at # set carry bit
    andi $at, Param0, 0x100 # read new carry bit
    andi Param0, Param0, 0xFF # set to 8 bits
    beq Param0, $zero, _GB_BITWISE_ADD_Z
    srl GB_F, $at, 4 # shift new carry bit into carry flag 
    jr $ra
    nop
_GB_BITWISE_ADD_Z:
    jr $ra
    set_flags Z_FLAG # set Z_FLAG
    
#######################
# Rotates Param0 1 bit right through carry and 
# sets flags
#######################

GB_RR_IMPL:
    sll $at, GB_F, 4                        # shift carry bit into position
    or Param0, Param0, $at                  # set carry bit
    sll $at, Param0, 4                      # shift new carry bit into pos
    andi GB_F, $at, 0x10                    # set carry bit

    srl Param0, Param0, 1                   # shift the bit once
    beq Param0, $zero, _GB_BITWISE_ADD_Z
    andi Param0, Param0, 0xFF               # set to 8 bits
    jr $ra
    nop

#######################
# Shifts param left one bit
#######################

GB_SLA_IMPL:
    sll Param0, Param0, 1
    srl GB_F, Param0, 4 # move carry bit into position
    andi Param0, Param0, 0xFF # mask result
    beqz Param0, _GB_BITWISE_ADD_Z # if zero set Z_FLAG
    andi GB_F, GB_F, C_FLAG # mask carry bit
    jr $ra
    nop
    
#######################
# Shifts param right 1 bit
#######################

GB_SRA_IMPL:
    sll GB_F, Param0, 4 # move carry bit into position
    sll Param0, Param0, 24 # shift left so right shift sign extends
    sra Param0, Param0, 25
    andi Param0, Param0, 0xFF # mask result
    beqz Param0, _GB_BITWISE_ADD_Z # if zero set Z_FLAG
    andi GB_F, GB_F, 0x10 # mask carry bit
    jr $ra
    nop
    
#######################
# Shifts param right 1 bit
#######################

GB_SRL_IMPL:
    sll GB_F, Param0, 4 # move carry bit into position
    srl Param0, Param0, 1
    andi Param0, Param0, 0xFF # mask result
    beqz Param0, _GB_BITWISE_ADD_Z # if zero set Z_FLAG
    andi GB_F, GB_F, 0x10 # mask carry bit
    jr $ra
    nop
    
#######################
# Swap nibbles Param0
#######################

GB_SWAP_IMPL:
    srl $at, Param0, 4 # move upper nibble down
    sll Param0, Param0, 4 # move lower nibble up
    or Param0, Param0, $at
    andi Param0, Param0, 0xFF # mask result
    beqz Param0, _GB_BITWISE_IMPL_IS_ZERO # if zero set Z_FLAG
    clear_flags Z_FLAG | N_FLAG | H_FLAG | C_FLAG
    jr $ra
    nop

#######################
# Adds Param0 to HL
#######################

_ADD_TO_HL:
    add GB_L, GB_L, Param0          # add to L
    srl $at, GB_L, 8                # get upper bits
    andi GB_L, GB_L, 0xFF           # mask to 8 bits
    add Param0, GB_H, $at           # add upper bits to h

    xor $at, Param0, $at            # determine half carry bit
    xor $at, GB_H, $at

    addi GB_H, Param0, 0            # store final result into GB_H

    andi $at, $at, 0x10             # mask carry bit
    sll $at, $at, 1                 # move carry bit into H flag position
    clear_flags N_FLAG | H_FLAG | C_FLAG
    or GB_F, GB_F, $at              # set half bit

    srl $at, GB_H, 4                # shift carry bit to C flag position
    andi $at, $at, 0x10             # mask carry bit
    or GB_F, GB_F, $at              # set carry git
    j DECODE_NEXT
    andi GB_F, GB_F, 0xFF           # mask to 8 bits

#######################
# Adds Param0 to SP
#######################

_ADD_TO_SP:
    add $at, Param0, GB_SP
    xor Param0, $at, Param0
    xor Param0, Param0, GB_SP # store half bit into Param0
    andi GB_SP, $at, 0xFFFF # move result into SP
    srl $at, $at, 12 # shift carry bit into position
    andi GB_F, $at, C_FLAG # move carry bit into position 
    srl Param0, Param0, 7 # move half flag into position
    andi Param0, Param0, H_FLAG # mask half flag
    j DECODE_NEXT
    or GB_F, GB_F, Param0 # set half flag

    
#######################
# Add Param0 + C to A
#######################

_ADC_TO_A:
    andi $at, GB_F, 0x10            # mask carry bit
    srl $at, $at, 4                 # move carry bit into first bit
    add TMP2, Param0, $at           # add carry bit
    add $at, GB_A, TMP2             # add to A
    xor Param0, Param0, GB_A
    xor Param0, Param0, $at         # calculate half carry
    andi GB_A, $at, 0xFF            # move result into A
    andi GB_F, $at, 0x100           # mask carry bit
    srl GB_F, GB_F, 4               # move carry bit into place
    sll Param0, Param0, 1           # shift half flag into position
    andi Param0, Param0, H_FLAG     # mask half carry bit
    bne GB_A, $zero, DECODE_NEXT    # check if z flag should be set
    or GB_F, GB_F, Param0           # set half carry bit
    j DECODE_NEXT
    set_flags Z_FLAG                # set z flag
    
#######################
# Add Param0 to A
#######################

_ADD_TO_A:
    add $at, GB_A, Param0           # add to A
    xor Param0, Param0, GB_A
    xor Param0, Param0, $at         # calculate half carry
    andi GB_A, $at, 0xFF            # move result into A
    andi GB_F, $at, 0x100           # mask carry bit
    srl GB_F, GB_F, 4               # move carry bit into place
    sll Param0, Param0, 1           # shift half flag into position
    andi Param0, Param0, H_FLAG     # mask half carry bit
    bne GB_A, $zero, DECODE_NEXT    # check if z flag should be set
    or GB_F, GB_F, Param0           # set half carry bit
    j DECODE_NEXT
    set_flags Z_FLAG                # set z flag

#######################
# Subtract Param0 + C from A
#######################

_SBC_FROM_A:
    andi $at, GB_F, 0x10            # mask carry bit
    srl $at, $at, 4                 # move carry bit into first bit
    add TMP2, Param0, $at           # add carry bit
    addi GB_A, GB_A, (C_FLAG | N_FLAG) << 4  # add carry bit and N flag to A
    sub $at, GB_A, TMP2
    xor Param0, Param0, GB_A
    xor Param0, Param0, $at
    sll Param0, Param0, 1           # shift half flag into position
    andi Param0, Param0, H_FLAG     # mask half carry bit
    andi GB_A, $at, 0xFF            # store and mask result
    andi $at, $at, 0xF00            # get the carry bit
    srl GB_F, $at, 4                # move c and n bits into flags
    xori GB_F, GB_F, C_FLAG         # negate C_FLAG
    bne GB_A, $zero, DECODE_NEXT    # check if z flag should be set
    or GB_F, GB_F, Param0           # set half carry bit
    j DECODE_NEXT
    set_flags Z_FLAG
    
#######################
# Subtract Param0 from A
#######################

_SUB_FROM_A:
    addi GB_A, GB_A, (C_FLAG | N_FLAG) << 4  # add carry bit and N flag to A
    sub $at, GB_A, Param0
    xor Param0, Param0, GB_A
    xor Param0, Param0, $at
    sll Param0, Param0, 1           # shift half flag into position
    andi Param0, Param0, H_FLAG     # mask half carry bit
    andi GB_A, $at, 0xFF            # store and mask result
    andi $at, $at, 0xF00            # get the carry bit
    srl GB_F, $at, 4                # move c and n bits into flags
    xori GB_F, GB_F, C_FLAG         # negate C_FLAG
    bne GB_A, $zero, DECODE_NEXT    # check if z flag should be set
    or GB_F, GB_F, Param0           # set half carry bit
    j DECODE_NEXT
    set_flags Z_FLAG
    
    
#######################
# Subtract Param0 to A
#######################

_CP_A:
    addi TMP2, GB_A, (C_FLAG | N_FLAG) << 4  # add carry bit and N flag to A
    sub $at, TMP2, Param0
    xor Param0, Param0, GB_A
    xor Param0, Param0, $at
    sll Param0, Param0, 1           # shift half flag into position
    andi Param0, Param0, H_FLAG     # mask half carry bit
    andi TMP2, $at, 0xFF            # store and mask result
    andi $at, $at, 0xF00            # get the carry bit
    srl GB_F, $at, 4                # move c and n bits into flags
    xori GB_F, GB_F, C_FLAG         # negate C_FLAG
    bne TMP2, $zero, DECODE_NEXT    # check if z flag should be set
    or GB_F, GB_F, Param0           # set half carry bit
    j DECODE_NEXT
    set_flags Z_FLAG

#######################
# Decimal encodes GB_A
#######################

_GB_DAA:
    clear_flags C_FLAG | Z_FLAG
    andi $at, GB_F, H_FLAG 
    bnez $at, _GB_DAA_ADJUST_LOW # if H_FLAG then adjust lower
    andi $at, GB_A, 0xF
    addi $at, $at, -9
    blez $at, _GB_DAA_HIGH_NIBBLE # if (A & 0xF <= 9) goto _GB_DAA_HIGH_NIBBLE 
    nop
_GB_DAA_ADJUST_LOW:
    set_flags H_FLAG
    addi GB_A, GB_A, 6
_GB_DAA_HIGH_NIBBLE:
    andi $at, GB_F, C_FLAG
    bnez $at, _GB_DAA_ADJUST_HIGH # if C_FLAG then adjust upper
    addi $at, GB_A, -0x99
    blez $at, DECODE_NEXT # if (A <= 0x99) goto next instruction
    nop
_GB_DAA_ADJUST_HIGH:
    set_flags C_FLAG
    j DECODE_NEXT
    addi GB_A, GB_A, 0x60