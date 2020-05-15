
.set noat      # allow manual use of $at
.set noreorder # don't insert nops after branches

#######################
# A0 CPUState
# A1 RAMPointer
# A2 InstructionCount
#
#######################

.set GB_A, $t0
.set GB_F, $t1
.set GB_B, $t2
.set GB_C, $t3
.set GB_D, $t4
.set GB_E, $t5
.set GB_H, $t6
.set GB_L, $t7
.set GB_SP, $t8
.set GB_PC, $t9

.set ADDR, $s0
.set VAL,  $s1
.set TMP2,  $s2

.set CPUState, $a0
.set MemoryStart, $a1
.set CycleCount, $a2

.set Param0, $a3

.eqv CPU_STATE_A, 0x0
.eqv CPU_STATE_F, 0x1
.eqv CPU_STATE_B, 0x2
.eqv CPU_STATE_C, 0x3
.eqv CPU_STATE_D, 0x4
.eqv CPU_STATE_E, 0x5
.eqv CPU_STATE_H, 0x6
.eqv CPU_STATE_L, 0x7
.eqv CPU_STATE_SP, 0x8
.eqv CPU_STATE_PC, 0xA
.eqv CPU_STATE_STOP_REASON, 0xC

.eqv Z_FLAG, 0x80
.eqv N_FLAG, 0x40
.eqv H_FLAG, 0x20
.eqv C_FLAG, 0x10

.eqv STACK_SIZE, 0x14
.eqv ST_RA, 0x0
.eqv ST_CYCLES_REQUESTED, 0x4
.eqv ST_S0, 0x8
.eqv ST_S1, 0xC
.eqv ST_S2, 0x10

.eqv CYCLES_PER_INSTR, 0x1

.eqv STOP_REASON_NONE, 0x0
.eqv STOP_REASON_STOP, 0x1
.eqv STOP_REASON_HALT, 0x2
.eqv STOP_REASON_INTERRUPT_RET, 0x3
.eqv STOP_REASON_ERROR, 0x4

.macro clear_flags flags
    andi GB_F, GB_F, %lo(~(\flags))
.endm

.macro set_flags flags
    ori GB_F, GB_F, \flags
.endm

.global runZ80CPU 
.balign 4 
runZ80CPU:
    addi $sp, $sp, -STACK_SIZE
    sw $ra, ST_RA($sp) # save return address
    sw CycleCount, ST_CYCLES_REQUESTED($sp) # save the original cycles to run 
    sw $s0, ST_S0($sp) # save whatever these are
    sw $s1, ST_S1($sp) # save whatever these are
    sw $s2, ST_S2($sp) # save whatever these are

    lbu GB_A, CPU_STATE_A(CPUState)
    lbu GB_F, CPU_STATE_F(CPUState)
    lbu GB_B, CPU_STATE_B(CPUState)
    lbu GB_C, CPU_STATE_C(CPUState)

    lbu GB_D, CPU_STATE_D(CPUState)
    lbu GB_E, CPU_STATE_E(CPUState)
    lbu GB_H, CPU_STATE_H(CPUState)
    lbu GB_L, CPU_STATE_L(CPUState)

    lhu GB_SP, CPU_STATE_SP(CPUState)
    lhu GB_PC, CPU_STATE_PC(CPUState)
    
    addi $at, $zero, STOP_REASON_NONE
    sb $at, CPU_STATE_STOP_REASON(CPUState)

DECODE_NEXT:
    blez CycleCount, GB_BREAK_LOOP
    nop
    jal READ_NEXT_INSTRUCTION # get the next instruction to decode
    nop

    la $at, GB_NOP # load start of jump table
    sll $v0, $v0, 5 # multiply address by 32 (4 bytes * 8 instructions)
    add $ra, $at, $v0
    jr $ra
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR
    #################
    # Each entry in the jump table needs
    # to be 8 instructions apart
    # any under must be padded with nops
#### 0x0X
GB_NOP: # start of jump table
    j DECODE_NEXT
    nop
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_BC_D16:
    jal READ_NEXT_INSTRUCTION # read immedate values
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    jal READ_NEXT_INSTRUCTION
    addi GB_B, $v0, 0 # store B
    j DECODE_NEXT
    addi GB_C, $v0, 0 # store C
    nop
    nop
GB_LD_BC_A:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    add VAL, GB_A, 0 # write the value to store
    sll ADDR, GB_B, 8 # write upper address
    j GB_DO_WRITE # call store subroutine
    or ADDR, ADDR, GB_C # write lower address
    nop
    nop
    nop
GB_INC_BC:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    addi GB_C, GB_C, 1 # incement the register
    srl $at, GB_C, 8
    andi GB_C, GB_C, 0xFF # keep at 8 bits
    add GB_B, GB_B, $at # add carry bit
    j DECODE_NEXT
    andi GB_B, GB_B, 0xFF # keep at 8 bits
    nop
GB_INC_B:
    jal GB_INC # call increment
    addi Param0, GB_B, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_B, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_DEC_B:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_B, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_B, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_LD_B_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_B, $v0, 0 #store value
    nop
    nop
    nop
    nop
GB_RLCA:
    jal GB_RLC_IMPL # do RLC
    addi Param0, GB_A, 0 # store A into param
    j DECODE_NEXT
    addi GB_A, Param0, 0 # store result back into A
    nop
    nop
    nop
    nop
GB_LD_A16_SP:
    jal READ_NEXT_INSTRUCTION # read immediate value
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 4 # update cycles run
    jal READ_NEXT_INSTRUCTION # read immediate value
    sll GB_SP, $v0, 8 #store upper address
    j DECODE_NEXT
    or GB_SP, GB_SP, $v0 #store lower address
    nop
    nop
GB_ADD_HL_BC:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    sll Param0, GB_B, 8 # load high order bits
    j _ADD_TO_HL
    or Param0, Param0, GB_C # load low order bits
    nop
    nop
    nop
    nop
GB_LD_A_BC:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_B, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_C # load lower address
    j DECODE_NEXT
    addi GB_A, $v0, 0 # store result into a
    nop
    nop
GB_DEC_BC:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    addi GB_C, GB_C, -1 # decrement C
    sra $at, GB_C, 8 # shift carry
    add GB_B, GB_B, $at  # add carry to b
    andi GB_C, GB_C, 0xFF # mask C
    j DECODE_NEXT
    andi GB_B, GB_B, 0xFF # mask B
    nop
GB_INC_C:
    jal GB_INC # call increment
    addi Param0, GB_C, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_C, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_DEC_C:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_C, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_C, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_LD_C_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_C, $v0, 0 #store value
    nop
    nop
    nop
    nop
GB_RRCA:
    jal GB_RRC_IMPL # call rotate
    addi Param0, GB_A, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_A, Param0, 0 # store paramter back
    nop
    nop
    nop
    nop
#### 0x1X
GB_STOP:
    jal READ_NEXT_INSTRUCTION # STOP will skip the next instruction
    nop
    addi $at, $zero, STOP_REASON_STOP
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
GB_LD_DE_D16:
    jal READ_NEXT_INSTRUCTION # read immedate values
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    jal READ_NEXT_INSTRUCTION
    addi GB_D, $v0, 0 # store D
    j DECODE_NEXT
    addi GB_E, $v0, 0 # store E
    nop
    nop
GB_LD_DE_A:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    add VAL, GB_A, 0 # write the value to store
    sll ADDR, GB_D, 8 # write upper address
    j GB_DO_WRITE # call store subroutine
    or ADDR, ADDR, GB_E # write lower address
    nop
    nop
    nop
GB_INC_DE:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    addi GB_E, GB_E, 1 # incement the register
    srl $at, GB_E, 8
    andi GB_E, GB_E, 0xFF # keep at 8 bits
    add GB_D, GB_D, $at # add carry bit
    j DECODE_NEXT
    andi GB_D, GB_D, 0xFF # keep at 8 bits
    nop
GB_INC_D:
    jal GB_INC # call increment
    addi Param0, GB_D, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_D, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_DEC_D:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_D, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_D, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_LD_D_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_D, $v0, 0 #store value
    nop
    nop
    nop
    nop
GB_RLA:
    jal GB_RL_IMPL # do RLC
    addi Param0, GB_A, 0 # store A into param
    j DECODE_NEXT
    addi GB_A, Param0, 0 # store result back into A
    nop
    nop
    nop
    nop
GB_JR:
    jal READ_NEXT_INSTRUCTION
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR  * 2 # update cycles run
    sll $v0, $v0, 24 # sign extend the bytes
    sra $v0, $v0, 24
    j DECODE_NEXT
    add GB_PC, GB_PC, $v0
    nop
    nop
GB_ADD_HL_DE:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    sll Param0, GB_D, 8 # load high order bits
    j _ADD_TO_HL
    or Param0, Param0, GB_E # load low order bits
    nop
    nop
    nop
    nop
GB_LD_A_DE:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_D, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_E # load lower address
    j DECODE_NEXT
    addi GB_A, $v0, 0 # store result into a
    nop
    nop
GB_DEC_DE:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    addi GB_E, GB_E, -1 # decrement E
    sra $at, GB_E, 8 # shift carry
    add GB_D, GB_D, $at  # add carry to D
    andi GB_E, GB_E, 0xFF # mask E
    j DECODE_NEXT
    andi GB_D, GB_D, 0xFF # mask D
    nop
GB_INC_E:
    jal GB_INC # call increment
    addi Param0, GB_E, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_E, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_DEC_E:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_E, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_E, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_LD_E_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_E, $v0, 0 #store value
    nop
    nop
    nop
    nop
GB_RRA:
    jal GB_RR_IMPL
    addi Param0, GB_A, 0
    j DECODE_NEXT
    addi GB_A, Param0, 0
    nop
    nop
    nop
    nop
### 0x2X
GB_JR_NZ:
    jal READ_NEXT_INSTRUCTION
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR  * 2 # update cycles run
    sll $v0, $v0, 24 # sign extend the bytes
    andi $at, GB_F, Z_FLAG # check z flag
    bne $at, $zero, DECODE_NEXT # skip jump if not zero
    sra $v0, $v0, 24 # second half of sign extend bits
    j DECODE_NEXT
    add GB_PC, GB_PC, $v0
GB_LD_HL_D16:
    jal READ_NEXT_INSTRUCTION # read immedate values
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    jal READ_NEXT_INSTRUCTION
    addi GB_H, $v0, 0 # store H
    j DECODE_NEXT
    addi GB_L, $v0, 0 # store L
    nop
    nop
GB_LDI_HL_A:
    add VAL, GB_A, 0 # write the value to store
    sll ADDR, GB_H, 8 # write upper address
    or ADDR, ADDR, GB_L # write lower address

    addi GB_L, ADDR, 1 # increment address
    srl GB_H, GB_L, 8 # store upper bits
    andi GB_L, GB_L, 0xFF # mask lower bits
    andi GB_H, GB_H, 0xFF # mask upper bits bits
    j GB_DO_WRITE # call store subroutine
    # intentially leave off nop to overflow to next instruction
GB_INC_HL:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    addi GB_L, GB_L, 1 # incement the register
    srl $at, GB_L, 8
    andi GB_L, GB_L, 0xFF # keep at 8 bits
    add GB_H, GB_H, $at # add carry bit
    j DECODE_NEXT
    andi GB_H, GB_H, 0xFF # keep at 8 bits
    nop
GB_INC_H:
    jal GB_INC # call increment
    addi Param0, GB_H, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_H, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_DEC_H:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_H, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_H, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_LD_H_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_H, $v0, 0 #store value
    nop
    nop
    nop
    nop
GB_DAA:
    j DECODE_NEXT
    nop
    nop
    nop
    nop
    nop
    nop
    nop
GB_JR_Z:
    jal READ_NEXT_INSTRUCTION
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR  * 2 # update cycles run
    sll $v0, $v0, 24 # sign extend the bytes
    andi $at, GB_F, Z_FLAG # check z flag
    beq $at, $zero, DECODE_NEXT # skip jump if zero
    sra $v0, $v0, 24 # second half of sign extend bits
    j DECODE_NEXT
    add GB_PC, GB_PC, $v0
GB_ADD_HL_HL:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    sll Param0, GB_H, 8 # load high order bits
    j _ADD_TO_HL
    or Param0, Param0, GB_L # load low order bits
_MASK_HL:
    andi GB_L, GB_L, 0xFF # mask lower bits
    andi GB_H, GB_H, 0xFF # mask upper bits bits
    j DECODE_NEXT
    nop
GB_LDI_A_HL:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    jal GB_DO_READ # call read instruction
    addi GB_L, ADDR, 1 # increment L
    addi GB_A, $v0, 0 # store result into a
    j _MASK_HL
    srl GB_H, GB_L, 8 # store incremented H
GB_DEC_HL:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    addi GB_L, GB_L, -1 # decrement E
    sra $at, GB_L, 8 # shift carry
    add GB_H, GB_H, $at  # add carry to D
    andi GB_L, GB_L, 0xFF # mask E
    j DECODE_NEXT
    andi GB_H, GB_H, 0xFF # mask D
    nop
GB_INC_L:
    jal GB_INC # call increment
    addi Param0, GB_L, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_L, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_DEC_L:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_L, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_L, Param0, 0 # move register back from call parameter
    nop
    nop
    nop
    nop
GB_LD_L_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_L, $v0, 0 #store value
    nop
    nop
    nop
    nop
GB_CPL:
    xori GB_A, GB_A, 0xFF
    j DECODE_NEXT
    set_flags N_FLAG | H_FLAG
    nop
    nop
    nop
    nop
    nop
### 0x3X
GB_JR_NC:
    jal READ_NEXT_INSTRUCTION
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR  * 2 # update cycles run
    sll $v0, $v0, 24 # sign extend the bytes
    andi $at, GB_F, C_FLAG # check c flag
    bne $at, $zero, DECODE_NEXT # skip jump if not zero
    sra $v0, $v0, 24 # second half of sign extend bits
    j DECODE_NEXT
    add GB_PC, GB_PC, $v0
GB_LD_SP_D16:
    jal READ_NEXT_INSTRUCTION # read immedate values
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    jal READ_NEXT_INSTRUCTION
    sll GB_SP, $v0, 8 # store high bits
    j DECODE_NEXT
    or GB_SP, GB_SP, $v0 # store low bits
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    addi GB_SP, GB_SP, 1
    j DECODE_NEXT
    andi GB_SP, GB_SP, 0xFFFF
    nop
    nop
    nop
    nop
GB_INC_HL_ADDR:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    sll ADDR, GB_H, 8 # write upper address
    jal GB_DO_READ
    or ADDR, ADDR, GB_L # write lower address
    jal GB_INC # call increment
    addi Param0, $v0, 0 # move loaded value to call parameter
    j DECODE_NEXT
    sb Param0, 0(ADDR) # use same ADDR calculated in GB_DO_READ
GB_DEC_HL_ADDR:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    sll ADDR, GB_H, 8 # write upper address
    jal GB_DO_READ
    or ADDR, ADDR, GB_L # write lower address
    jal GB_DEC # call decrement
    addi Param0, $v0, 0 # move loaded value to call parameter
    j DECODE_NEXT
    sb Param0, 0(ADDR) # use same ADDR calculated in GB_DO_READ
GB_LD_HL_ADDR_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
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
    jal READ_NEXT_INSTRUCTION
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR  * 2 # update cycles run
    sll $v0, $v0, 24 # sign extend the bytes
    andi $at, GB_F, C_FLAG # check c flag
    beq $at, $zero, DECODE_NEXT # skip jump if zero
    sra $v0, $v0, 24 # second half of sign extend bits
    j DECODE_NEXT
    add GB_PC, GB_PC, $v0
GB_ADD_HL_SP:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    j _ADD_TO_HL
    addi Param0, GB_SP, 0
    nop
    nop
    nop
    nop
    nop
GB_LDD_A_HL:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    jal GB_DO_READ # call read instruction
    addi GB_L, ADDR, -1 # decrement L
    addi GB_A, $v0, 0 # store result into a
    j _MASK_HL
    srl GB_H, GB_L, 8 # store incremented H
GB_DEC_SP:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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

######################
# Return instructions left to run
######################
GB_BREAK_LOOP:
    lw $v0, ST_CYCLES_REQUESTED($sp) # retrieve original cycles requested
    sub $v0, $v0, CycleCount # calculate and return the number of cycles run
_GB_EXIT_EARLY:
    sb GB_A, CPU_STATE_A(CPUState)
    sb GB_F, CPU_STATE_F(CPUState)
    sb GB_B, CPU_STATE_B(CPUState)
    sb GB_C, CPU_STATE_C(CPUState)

    sb GB_D, CPU_STATE_D(CPUState)
    sb GB_E, CPU_STATE_E(CPUState)
    sb GB_H, CPU_STATE_H(CPUState)
    sb GB_L, CPU_STATE_L(CPUState)

    sh GB_SP, CPU_STATE_SP(CPUState)
    sh GB_PC, CPU_STATE_PC(CPUState)
    
    lw $s0, ST_S0($sp) # restore whatever these are
    lw $s1, ST_S1($sp) # restore whatever these are
    lw $s2, ST_S2($sp) # restore whatever these are
    lw $ra, ST_RA($sp) # load return address
    jr $ra
    addi $sp, $sp, STACK_SIZE

######################
# Writes VAL to ADDR
######################

GB_DO_WRITE:
    addiu $at, $zero, 0x8000
    sub $at, ADDR, $at
    bgez $at, _GB_DO_WRITE # if ADDR >= 0x8000 just write
    nop 
    # todo do bank switching
    j DECODE_NEXT
    nop
_GB_DO_WRITE:
    srl $at, ADDR, 9 # load bank in $at
    andi ADDR, ADDR, 0x1FF # keep offset in ADDR
    sll $at, $at, 2 # word align the memory map offset
    add $at, $at, MemoryStart # lookup start of bank in array at MemoryStart
    lw $at, 0($at) # load start of memory bank
    add ADDR, ADDR, $at # use address relative to memory bank
    j DECODE_NEXT
    sb VAL, 0(ADDR) # store the byte
    
######################
# Reads ADDR into $v0
######################

GB_DO_READ:
    srl $at, ADDR, 9 # load bank in $at
    andi ADDR, ADDR, 0x1FF # keep offset in ADDR
    sll $at, $at, 2 # word align the memory map offset
    add $at, $at, MemoryStart # lookup start of bank in array at MemoryStart
    lw $at, 0($at) # load start of memory bank
    add ADDR, ADDR, $at # use address relative to memory bank
    jr $ra
    lbu $v0, 0(ADDR) # load the byte

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
    beq Param0, $zero, _GB_RLC_IMPL_IS_ZERO
    sll Param0, Param0, 1 # shift the bit once
    srl $at, Param0, 8 # shift carry bit back
    or Param0, Param0, $at # put rotated bit back
    andi Param0, Param0, 0xFF # keep param 8 bits
    jr $ra
    sll GB_F, $at, 4 # shift carry bit into C_FLAG position
_GB_RLC_IMPL_IS_ZERO:
    jr $ra
    andi GB_F, $zero, Z_FLAG # set Z_FLAG

#######################
# Rotates Param0 1 bit left and 
# sets flags
#######################

GB_RRC_IMPL:
    beq Param0, $zero, _GB_RRC_IMPL_IS_ZERO
    sll $at, Param0, 7 # shift carry bit back
    srl Param0, Param0, 1 # shift bit once
    or Param0, Param0, $at # put rotated bit back
    andi Param0, Param0, 0xFF # keep param 8 bits
    andi $at, $at, 0x80 # mask carry bit
    jr $ra
    srl GB_F, $at, 3 # move carry bit into C_FLAG

_GB_RRC_IMPL_IS_ZERO:
    jr $ra
    andi GB_F, $zero, Z_FLAG # set Z_FLAG
    
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
    beq Param0, $zero, _GB_RL_IMPL_IS_ZERO
    srl GB_F, $at, 4 # shift new carry bit into carry flag 
    jr $ra
    nop
_GB_RL_IMPL_IS_ZERO:
    jr $ra
    set_flags Z_FLAG # set Z_FLAG
    
#######################
# Rotates Param0 1 bit right through carry and 
# sets flags
#######################

GB_RR_IMPL:
    sll $at, GB_F, 4 # shift carry bit into position
    or Param0, Param0, $at # set carry bit
    sll $at, Param0, 4 # shift new carry bit into pos
    andi GB_F, $at, 0x10 # set carry bit

    srl Param0, Param0, 1 # shift the bit once
    beq Param0, $zero, _GB_RR_IMPL_IS_ZERO
    andi Param0, Param0, 0xFF # set to 8 bits
    jr $ra
    nop
_GB_RR_IMPL_IS_ZERO:
    jr $ra
    set_flags Z_FLAG # set Z_FLAG

#######################
# Adds Param0 to HL
#######################

_ADD_TO_HL:
    add GB_L, GB_L, Param0 # add to L
    srl $at, GB_L, 8 # get upper bits
    andi GB_L, GB_L, 0xFF # mask to 8 bits
    add Param0, GB_H, $at # add upper bits to h

    xor $at, Param0, $at # determine half carry bit
    xor $at, GB_H, $at

    addi GB_H, Param0, 0 # store final result into GB_H

    andi $at, $at, 0x10 # mask carry bit
    sll $at, $at, 1 # move carry bit into H flag position
    clear_flags N_FLAG | H_FLAG | C_FLAG
    or GB_F, GB_F, $at # set half bit

    srl $at, GB_H, 4 # shift carry bit to C flag position
    andi $at, $at, 0x10 # mask carry bit
    or GB_F, GB_F, $at # set carry git
    j DECODE_NEXT
    andi GB_F, GB_F, 0xFF # mask to 8 bits

#######################
# Decimal encodes Param0
#######################

GB_DA:
    andi $at, Param0, 0x0F
    jr $ra
    nop


#######################
# Reads the byte at PC
# increments the program counter
# then returns the byte in register $v0
#######################

READ_NEXT_INSTRUCTION:
    # read at PC, increment PC, return value
    addi ADDR, GB_PC, 0
    j GB_DO_READ
    addi GB_PC, GB_PC, 1
