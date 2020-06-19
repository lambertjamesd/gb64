

#######################
# Handle the CB_PREFIX command
#######################

_GB_PREFIX_CB:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    move TMP2, $v0
    andi $at, $v0, 0x7
    sll $at, $at, 4
    la $ra, _GB_PREFIX_DECODE_REGISTER # load jump table address
    add $ra, $ra, $at # calculate jump offset
    jr $ra # jump to instruction decode
    nop
_GB_PREFIX_DECODE_REGISTER:
    jal _GB_PREFIX_DECODE_INSTRUCTION
    move Param0, GB_B
    j DECODE_NEXT
    move GB_B, Param0
    
    jal _GB_PREFIX_DECODE_INSTRUCTION
    move Param0, GB_C
    j DECODE_NEXT
    move GB_C, Param0
    
    jal _GB_PREFIX_DECODE_INSTRUCTION
    move Param0, GB_D
    j DECODE_NEXT
    move GB_D, Param0
    
    jal _GB_PREFIX_DECODE_INSTRUCTION
    move Param0, GB_E
    j DECODE_NEXT
    move GB_E, Param0
    
    jal _GB_PREFIX_DECODE_INSTRUCTION
    move Param0, GB_H
    j DECODE_NEXT
    move GB_H, Param0
    
    jal _GB_PREFIX_DECODE_INSTRUCTION
    move Param0, GB_L
    j DECODE_NEXT
    move GB_L, Param0
    
    j _GB_PREFIX_DECODE_HL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 1 # update cycles run
    nop
    nop
    
    jal _GB_PREFIX_DECODE_INSTRUCTION
    move Param0, GB_A
    j DECODE_NEXT
    move GB_A, Param0

_GB_PREFIX_DECODE_HL:
    sll ADDR, GB_H, 8
    jal GB_DO_READ
    or ADDR, ADDR, GB_L
    jal _GB_PREFIX_DECODE_INSTRUCTION
    move Param0, $v0
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 1 # update cycles run separate. This allows the bit instructions to take 3 cycles
    sll ADDR, GB_H, 8
    or ADDR, ADDR, GB_L
    j GB_DO_WRITE
    move VAL, Param0

_GB_PREFIX_DECODE_INSTRUCTION:
    andi $at, TMP2, 0xF8 # calculate instruction
    la TMP3, _GB_PREFIX_RLC
    add TMP3, TMP3, $at # calculate relative jump
    jr TMP3 #jump in table
    nop
_GB_PREFIX_RLC:
    j GB_RLC_IMPL
    nop
_GB_PREFIX_RRC:
    j GB_RRC_IMPL
    nop
_GB_PREFIX_RL:
    j GB_RL_IMPL
    nop
_GB_PREFIX_RR:
    j GB_RR_IMPL
    nop
_GB_PREFIX_SLA:
    j GB_SLA_IMPL
    nop
_GB_PREFIX_SRA:
    j GB_SRA_IMPL
    nop
_GB_PREFIX_SWAP:
    j GB_SWAP_IMPL
    nop
_GB_PREFIX_SRL:
    j GB_SRL_IMPL
    nop
_GB_PREFIX_BIT_0:
    j _GB_PREFIX_FINISH_BIT
    sll Param0, Param0, 7
_GB_PREFIX_BIT_1:
    j _GB_PREFIX_FINISH_BIT
    sll Param0, Param0, 6
_GB_PREFIX_BIT_2:
    j _GB_PREFIX_FINISH_BIT
    sll Param0, Param0, 5
_GB_PREFIX_BIT_3:
    j _GB_PREFIX_FINISH_BIT
    sll Param0, Param0, 4
_GB_PREFIX_BIT_4:
    j _GB_PREFIX_FINISH_BIT
    sll Param0, Param0, 3
_GB_PREFIX_BIT_5:
    j _GB_PREFIX_FINISH_BIT
    sll Param0, Param0, 2
_GB_PREFIX_BIT_6:
    j _GB_PREFIX_FINISH_BIT
    sll Param0, Param0, 1
_GB_PREFIX_BIT_7:
    j _GB_PREFIX_FINISH_BIT
    nop
_GB_RES_BIT_0:
    jr $ra
    andi Param0, Param0, 0xFE
_GB_RES_BIT_1:
    jr $ra
    andi Param0, Param0, 0xFD
_GB_RES_BIT_2:
    jr $ra
    andi Param0, Param0, 0xFB
_GB_RES_BIT_3:
    jr $ra
    andi Param0, Param0, 0xF7
_GB_RES_BIT_4:
    jr $ra
    andi Param0, Param0, 0xEF
_GB_RES_BIT_5:
    jr $ra
    andi Param0, Param0, 0xDF
_GB_RES_BIT_6:
    jr $ra
    andi Param0, Param0, 0xBF
_GB_RES_BIT_7:
    jr $ra
    andi Param0, Param0, 0x7F
_GB_SET_BIT_0:
    jr $ra
    ori Param0, Param0, 0x01
_GB_SET_BIT_1:
    jr $ra
    ori Param0, Param0, 0x02
_GB_SET_BIT_2:
    jr $ra
    ori Param0, Param0, 0x04
_GB_SET_BIT_3:
    jr $ra
    ori Param0, Param0, 0x08
_GB_SET_BIT_4:
    jr $ra
    ori Param0, Param0, 0x10
_GB_SET_BIT_5:
    jr $ra
    ori Param0, Param0, 0x20
_GB_SET_BIT_6:
    jr $ra
    ori Param0, Param0, 0x40
_GB_SET_BIT_7:
    jr $ra
    ori Param0, Param0, 0x80

_GB_PREFIX_FINISH_BIT:
    clear_flags Z_FLAG | N_FLAG
    andi Param0, Param0, Z_FLAG # clear all but the z flag position
    xori Param0, Param0, Z_FLAG # negate the z flag
    or GB_F, GB_F, Param0
    j DECODE_NEXT # don't jr since bit checks don't need to store back
    set_flags H_FLAG # set h flag
    