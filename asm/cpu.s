
.set noat      # allow manual use of $at
.set noreorder # don't insert nops after branches

.include "asm/registers.inc"
.include "asm/memory.inc"

.macro clear_flags flags
    andi GB_F, GB_F, %lo(~(\flags))
.endm

.macro set_flags flags
    ori GB_F, GB_F, \flags
.endm

.global runCPU 
.balign 4 
runCPU:
    addi $sp, $sp, -STACK_SIZE
    sw $ra, ST_RA($sp) # save return address
    sw $s0, ST_S0($sp) # save caller registers
    sw $s1, ST_S1($sp) 
    sw $s2, ST_S2($sp) 
    sw $s3, ST_S3($sp) 
    sw $s4, ST_S4($sp) 
    sw $s5, ST_S5($sp) 
    sw $s6, ST_S6($sp) 
    sw $s7, ST_S7($sp) 

    lbu GB_A, CPU_STATE_A(CPUState)
    lbu GB_F, CPU_STATE_F(CPUState)
    lbu GB_B, CPU_STATE_B(CPUState)
    lbu GB_C, CPU_STATE_C(CPUState)

    lbu GB_D, CPU_STATE_D(CPUState)
    lbu GB_E, CPU_STATE_E(CPUState)
    lbu GB_H, CPU_STATE_H(CPUState)
    lbu GB_L, CPU_STATE_L(CPUState)

    lhu GB_SP, CPU_STATE_SP(CPUState)
    lhu Param0, CPU_STATE_PC(CPUState)

    jal SET_GB_PC
    # make sure GB_PC doesn't match Param0 to force bank load
    xori GB_PC, Param0, 0xFFFF 

    # load timer
    lw CYCLES_RUN, CPU_STATE_CYCLES_RUN(CPUState)

    add $at, CycleTo, CYCLES_RUN    # calculate upper bound of execution
    sw $at, ST_CYCLE_TO($sp)
    
    lbu $at, CPU_STATE_STOP_REASON(CPUState)
    bnez $at, GB_SIMULATE_HALTED

    jal CALCULATE_NEXT_STOPPING_POINT
    nop

    la TMP4, 0x80700000

DECODE_NEXT:
    sltu $at, CYCLES_RUN, CycleTo
    beq $at, $zero, HANDLE_STOPPING_POINT
    nop

    jal READ_NEXT_INSTRUCTION # get the next instruction to decode
    nop

    la $at, 0x80700000 - 4
    sw Memory, -4($at)
    sw TMP4, 0($at)
    sb $v0, 0(TMP4)
    sh GB_PC, 2(TMP4)
    addi TMP4, TMP4, 4

    la $at, 0x80800000
    sltu $at, TMP4, $at
    bne $at, $zero, _DEBUG_SKIP
    nop
    la TMP4, 0x80700000

_DEBUG_SKIP:

    la $at, GB_NOP # load start of jump table
    sll $v0, $v0, 5 # multiply address by 32 (4 bytes * 8 instructions)
    add $ra, $at, $v0
    jr $ra
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR
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
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    jal READ_NEXT_INSTRUCTION
    addi GB_C, $v0, 0 # store C
    j DECODE_NEXT
    addi GB_B, $v0, 0 # store B
    nop
    nop
GB_LD_BC_A:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    add VAL, GB_A, 0 # write the value to store
    sll ADDR, GB_B, 8 # write upper address
    j GB_DO_WRITE # call store subroutine
    or ADDR, ADDR, GB_C # write lower address
    nop
    nop
    nop
GB_INC_BC:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
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
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
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
    jal READ_NEXT_INSTRUCTION_16
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 4 # update cycles run
    move ADDR, $v0 # use immediate address
    j GB_DO_WRITE_16
    addi VAL, GB_SP, 0 # store value to write
    nop
    nop
    nop
GB_ADD_HL_BC:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll Param0, GB_B, 8 # load high order bits
    j _ADD_TO_HL
    or Param0, Param0, GB_C # load low order bits
    nop
    nop
    nop
    nop
GB_LD_A_BC:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_B, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_C # load lower address
    j DECODE_NEXT
    addi GB_A, $v0, 0 # store result into a
    nop
    nop
GB_DEC_BC:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
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
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
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
    j GB_SIMULATE_HALTED # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
GB_LD_DE_D16:
    jal READ_NEXT_INSTRUCTION # read immedate values
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    jal READ_NEXT_INSTRUCTION
    addi GB_E, $v0, 0 # store E
    j DECODE_NEXT
    addi GB_D, $v0, 0 # store D
    nop
    nop
GB_LD_DE_A:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    add VAL, GB_A, 0 # write the value to store
    sll ADDR, GB_D, 8 # write upper address
    j GB_DO_WRITE # call store subroutine
    or ADDR, ADDR, GB_E # write lower address
    nop
    nop
    nop
GB_INC_DE:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
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
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
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
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR  * 2 # update cycles run
    sll $v0, $v0, 24 # sign extend the bytes
    sra $v0, $v0, 24
    jal SET_GB_PC
    add Param0, GB_PC, $v0
    j DECODE_NEXT
    nop
GB_ADD_HL_DE:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll Param0, GB_D, 8 # load high order bits
    j _ADD_TO_HL
    or Param0, Param0, GB_E # load low order bits
    nop
    nop
    nop
    nop
GB_LD_A_DE:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_D, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_E # load lower address
    j DECODE_NEXT
    addi GB_A, $v0, 0 # store result into a
    nop
    nop
GB_DEC_DE:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
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
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
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
    andi $at, GB_F, Z_FLAG # check z flag
    bne $at, $zero, _SKIP_JR # if Z_FLAG != 0 skip jump
    nop
    j GB_JR
    nop
    nop
    nop
    nop
GB_LD_HL_D16:
    jal READ_NEXT_INSTRUCTION # read immedate values
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    jal READ_NEXT_INSTRUCTION
    addi GB_L, $v0, 0 # store H
    j DECODE_NEXT
    addi GB_H, $v0, 0 # store L
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
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
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
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_H, $v0, 0 #store value
    nop
    nop
    nop
    nop
GB_DAA:
    j _GB_DAA
    nop
    nop
    nop
    nop
    nop
    nop
    nop
GB_JR_Z:
    andi $at, GB_F, Z_FLAG # check z flag
    beq $at, $zero, _SKIP_JR # skip jump if not zero
    nop
    j GB_JR
    nop
    nop
    nop
    nop
GB_ADD_HL_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll Param0, GB_H, 8 # load high order bits
    j _ADD_TO_HL
    or Param0, Param0, GB_L # load low order bits
_MASK_HL:
    andi GB_L, GB_L, 0xFF # mask lower bits
    andi GB_H, GB_H, 0xFF # mask upper bits bits
    j DECODE_NEXT
    nop
GB_LDI_A_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    jal GB_DO_READ # call read instruction
    addi GB_L, ADDR, 1 # increment L
    addi GB_A, $v0, 0 # store result into a
    j _MASK_HL
    srl GB_H, GB_L, 8 # store incremented H
GB_DEC_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
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
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
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
    addi Param0, GB_SP, 0
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
### 0x4X
GB_LD_B_B:
    j DECODE_NEXT
    nop
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_B_C:
    j DECODE_NEXT
    addi GB_B, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_B_D:
    j DECODE_NEXT
    addi GB_B, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_B_E:
    j DECODE_NEXT
    addi GB_B, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_B_H:
    j DECODE_NEXT
    addi GB_B, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_B_L:
    j DECODE_NEXT
    addi GB_B, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_B_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j DECODE_NEXT
    addi GB_B, $v0, 0
    nop
    nop
GB_LD_B_A:
    j DECODE_NEXT
    addi GB_B, GB_A, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_C_B:
    j DECODE_NEXT
    addi GB_C, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_C_C:
    j DECODE_NEXT
    nop
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_C_D:
    j DECODE_NEXT
    addi GB_C, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_C_E:
    j DECODE_NEXT
    addi GB_C, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_C_H:
    j DECODE_NEXT
    addi GB_C, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_C_L:
    j DECODE_NEXT
    addi GB_C, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_C_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j DECODE_NEXT
    addi GB_C, $v0, 0
    nop
    nop
GB_LD_C_A:
    j DECODE_NEXT
    addi GB_C, GB_A, 0
    nop
    nop
    nop
    nop
    nop
    nop
### 0x5X
GB_LD_D_B:
    j DECODE_NEXT
    addi GB_D, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_D_C:
    j DECODE_NEXT
    addi GB_D, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_D_D:
    j DECODE_NEXT
    addi GB_D, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_D_E:
    j DECODE_NEXT
    addi GB_D, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_D_H:
    j DECODE_NEXT
    addi GB_D, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_D_L:
    j DECODE_NEXT
    addi GB_D, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_D_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j DECODE_NEXT
    addi GB_D, $v0, 0
    nop
    nop
GB_LD_D_A:
    j DECODE_NEXT
    addi GB_D, GB_A, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_E_B:
    j DECODE_NEXT
    addi GB_E, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_E_C:
    j DECODE_NEXT
    addi GB_E, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_E_D:
    j DECODE_NEXT
    addi GB_E, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_E_E:
    j DECODE_NEXT
    addi GB_E, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_E_H:
    j DECODE_NEXT
    addi GB_E, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_E_L:
    j DECODE_NEXT
    addi GB_E, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_E_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j DECODE_NEXT
    addi GB_E, $v0, 0
    nop
    nop
GB_LD_E_A:
    j DECODE_NEXT
    addi GB_E, GB_A, 0
    nop
    nop
    nop
    nop
    nop
    nop
### 0x6X
GB_LD_H_B:
    j DECODE_NEXT
    addi GB_H, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_H_C:
    j DECODE_NEXT
    addi GB_H, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_H_D:
    j DECODE_NEXT
    addi GB_H, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_H_E:
    j DECODE_NEXT
    addi GB_H, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_H_H:
    j DECODE_NEXT
    addi GB_H, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_H_L:
    j DECODE_NEXT
    addi GB_H, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_H_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j DECODE_NEXT
    addi GB_H, $v0, 0
    nop
    nop
GB_LD_H_A:
    j DECODE_NEXT
    addi GB_H, GB_A, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_L_B:
    j DECODE_NEXT
    addi GB_L, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_L_C:
    j DECODE_NEXT
    addi GB_L, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_L_D:
    j DECODE_NEXT
    addi GB_L, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_L_E:
    j DECODE_NEXT
    addi GB_L, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_L_H:
    j DECODE_NEXT
    addi GB_L, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_L_L:
    j DECODE_NEXT
    addi GB_L, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_L_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j DECODE_NEXT
    addi GB_L, $v0, 0
    nop
    nop
GB_LD_L_A:
    j DECODE_NEXT
    addi GB_L, GB_A, 0
    nop
    nop
    nop
    nop
    nop
    nop
### 0x7X
GB_LD_HL_B:    
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_B, 0
    nop
    nop
    nop
GB_LD_HL_C:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_C, 0
    nop
    nop
    nop
GB_LD_HL_D:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_D, 0
    nop
    nop
    nop
GB_LD_HL_E:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_E, 0
    nop
    nop
    nop
GB_LD_HL_H:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_H, 0
    nop
    nop
    nop
GB_LD_HL_L:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_L, 0
    nop
    nop
    nop
GB_HALT:
    addi $at, $zero, STOP_REASON_HALT
    j GB_SIMULATE_HALTED
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_LD_HL_A:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_A, 0
    nop
    nop
    nop
GB_LD_A_B:
    j DECODE_NEXT
    addi GB_A, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_A_C:
    j DECODE_NEXT
    addi GB_A, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_A_D:
    j DECODE_NEXT
    addi GB_A, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_A_E:
    j DECODE_NEXT
    addi GB_A, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_A_H:
    j DECODE_NEXT
    addi GB_A, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_A_L:
    j DECODE_NEXT
    addi GB_A, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_LD_A_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j DECODE_NEXT
    addi GB_A, $v0, 0
    nop
    nop
GB_LD_A_A:
    j DECODE_NEXT
    addi GB_A, GB_A, 0
    nop
    nop
    nop
    nop
    nop
    nop
### 0x8X
GB_ADD_A_B:
    j _ADD_TO_A
    addi Param0, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_ADD_A_C:
    j _ADD_TO_A
    addi Param0, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_ADD_A_D:
    j _ADD_TO_A
    addi Param0, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_ADD_A_E:
    j _ADD_TO_A
    addi Param0, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_ADD_A_H:
    j _ADD_TO_A
    addi Param0, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_ADD_A_L:
    j _ADD_TO_A
    addi Param0, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_ADD_A_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j _ADD_TO_A
    addi Param0, $v0, 0
    nop
    nop
GB_ADD_A_A:
    j _ADD_TO_A
    addi Param0, GB_A, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_ADC_A_B:
    j _ADC_TO_A
    addi Param0, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_ADC_A_C:
    j _ADC_TO_A
    addi Param0, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_ADC_A_D:
    j _ADC_TO_A
    addi Param0, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_ADC_A_E:
    j _ADC_TO_A
    addi Param0, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_ADC_A_H:
    j _ADC_TO_A
    addi Param0, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_ADC_A_L:
    j _ADC_TO_A
    addi Param0, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_ADC_A_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j _ADC_TO_A
    addi Param0, $v0, 0
    nop
    nop
GB_ADC_A_A:
    j _ADC_TO_A
    addi Param0, GB_A, 0
    nop
    nop
    nop
    nop
    nop
    nop
### 0x9X
GB_SUB_B:
    j _SUB_FROM_A
    addi Param0, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SUB_C:
    j _SUB_FROM_A
    addi Param0, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SUB_D:
    j _SUB_FROM_A
    addi Param0, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SUB_E:
    j _SUB_FROM_A
    addi Param0, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SUB_H:
    j _SUB_FROM_A
    addi Param0, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SUB_L:
    j _SUB_FROM_A
    addi Param0, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SUB_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j _SUB_FROM_A
    addi Param0, $v0, 0
    nop
    nop
GB_SUB_A:
    addi GB_A, $zero, 0
    j DECODE_NEXT
    addi GB_F, $zero, Z_FLAG | N_FLAG
    nop
    nop
    nop
    nop
    nop
GB_SBC_B:
    j _SBC_FROM_A
    addi Param0, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SBC_C:
    j _SBC_FROM_A
    addi Param0, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SBC_D:
    j _SBC_FROM_A
    addi Param0, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SBC_E:
    j _SBC_FROM_A
    addi Param0, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SBC_H:
    j _SBC_FROM_A
    addi Param0, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SBC_L:
    j _SBC_FROM_A
    addi Param0, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_SBC_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j _SBC_FROM_A
    addi Param0, $v0, 0
    nop
    nop
GB_SBC_A:
    j _SBC_FROM_A
    addi Param0, GB_A, 0
    nop
    nop
    nop
    nop
    nop
    nop
### 0xAX
GB_AND_B:
    and GB_A, GB_A, GB_B
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_AND_C:
    and GB_A, GB_A, GB_C
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_AND_D:
    and GB_A, GB_A, GB_D
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_AND_E:
    and GB_A, GB_A, GB_E
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_AND_H:
    and GB_A, GB_A, GB_H
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_AND_L:
    and GB_A, GB_A, GB_L
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_AND_HL:
    jal READ_HL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    and GB_A, GB_A, $v0
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
GB_AND_A:
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
    nop
GB_XOR_B:
    xor GB_A, GB_A, GB_B
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_XOR_C:
    xor GB_A, GB_A, GB_C
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_XOR_D:
    xor GB_A, GB_A, GB_D
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_XOR_E:
    xor GB_A, GB_A, GB_E
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_XOR_H:
    xor GB_A, GB_A, GB_H
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_XOR_L:
    xor GB_A, GB_A, GB_L
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_XOR_HL:
    jal READ_HL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    xor GB_A, GB_A, $v0
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
GB_XOR_A:
    addi GB_A, $zero, 0
    j DECODE_NEXT
    addi GB_F, GB_F, Z_FLAG
    nop
    nop
    nop
    nop
    nop
# 0xBX
GB_OR_B:
    or GB_A, GB_A, GB_B
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_OR_C:
    or GB_A, GB_A, GB_C
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_OR_D:
    or GB_A, GB_A, GB_D
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_OR_E:
    or GB_A, GB_A, GB_E
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_OR_H:
    or GB_A, GB_A, GB_H
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_OR_L:
    or GB_A, GB_A, GB_L
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
GB_OR_HL:
    jal READ_HL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    or GB_A, GB_A, $v0
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
GB_OR_A:
    bne GB_A, $zero, DECODE_NEXT
    addi GB_F, GB_F, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
    nop
    nop
    nop
GB_CP_B:
    j _CP_A
    addi Param0, GB_B, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_CP_C:
    j _CP_A
    addi Param0, GB_C, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_CP_D:
    j _CP_A
    addi Param0, GB_D, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_CP_E:
    j _CP_A
    addi Param0, GB_E, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_CP_H:
    j _CP_A
    addi Param0, GB_H, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_CP_L:
    j _CP_A
    addi Param0, GB_L, 0
    nop
    nop
    nop
    nop
    nop
    nop
GB_CP_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j _CP_A
    addi Param0, $v0, 0
    nop
    nop
GB_CP_A:
    j DECODE_NEXT
    addi GB_F, $zero, Z_FLAG | N_FLAG
_SKIP_JP:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    addi Param0, GB_PC, 2
    jal SET_GB_PC
    andi Param0, Param0, 0xFFFF
    j DECODE_NEXT
    nop
### 0xCX
GB_RET_NZ:
    andi $at, GB_F, Z_FLAG
    bne $at, $zero, DECODE_NEXT # if Z_FLAG != 0 skip return
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _GB_RET
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    nop
    nop
    nop
GB_POP_BC:
    addi ADDR, GB_SP, 0
    jal GB_DO_READ_16
    addi GB_SP, GB_SP, 2
    andi GB_SP, GB_SP, 0xFFFF
    srl GB_B, $v0, 8 # store B
    andi GB_C, $v0, 0xFF # store C
    j DECODE_NEXT
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
GB_JP_NZ:
    andi $at, GB_F, Z_FLAG
    bne $at, $zero, _SKIP_JP # if Z_FLAG != 0 skip jump
    nop
    jal GB_JP
    nop
    nop
    nop
    nop
GB_JP:
    jal READ_NEXT_INSTRUCTION_16
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    jal SET_GB_PC
    move Param0, $v0
    j DECODE_NEXT
    nop
    nop
    nop
GB_CALL_NZ:
    andi $at, GB_F, Z_FLAG
    bne $at, $zero, _SKIP_JP # if Z_FLAG != 0 skip the call
    nop
    j _GB_CALL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 4 # update cycles run
    nop
    nop
    nop
GB_PUSH_BC:
    addi GB_SP, GB_SP, -2
    andi GB_SP, GB_SP, 0xFFFF
    addi ADDR, GB_SP, 0
    sll VAL, GB_B, 8
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 3 # update cycles run
    j GB_DO_WRITE_16
    or VAL, VAL, GB_C
    nop
GB_ADD_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _ADD_TO_A
    addi Param0, $v0, 0
    nop
    nop
    nop
    nop
GB_RST_00H:
    addi GB_SP, GB_SP, -2
    andi ADDR, GB_SP, 0xFFFF
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    jal SET_GB_PC
    addi Param0, $zero, 0x0000
    j GB_DO_WRITE_16
    move GB_SP, ADDR
GB_RET_Z:
    andi $at, GB_F, Z_FLAG
    beq $at, $zero, DECODE_NEXT # if Z_FLAG == 0 skip RET
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _GB_RET
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    nop
    nop
    nop
GB_RET:
    j _GB_RET
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    nop
    nop
    nop
    nop
    nop
    nop
GB_JP_Z:
    andi $at, GB_F, Z_FLAG
    beq $at, $zero, _SKIP_JP # if Z_FLAG == 0 skip jump
    nop
    jal GB_JP
    nop
    nop
    nop
    nop
GB_PREFIX_CB:
    j _GB_PREFIX_CB
    nop
    nop
    nop
    nop
    nop
    nop
    nop
GB_CALL_Z:
    andi $at, GB_F, Z_FLAG
    beq $at, $zero, _SKIP_JP # if Z_FLAG == 0 skip call
    nop
    j _GB_CALL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 4 # update cycles run
    nop
    nop
    nop
GB_CALL:
    j _GB_CALL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 4 # update cycles run
    nop
    nop
    nop
    nop
    nop
    nop
GB_ADC_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _ADC_TO_A
    addi Param0, $v0, 0
    nop
    nop
    nop
    nop
GB_RST_08H:
    addi GB_SP, GB_SP, -2
    andi ADDR, GB_SP, 0xFFFF
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    jal SET_GB_PC
    addi Param0, $zero, 0x0008
    j GB_DO_WRITE_16
    move GB_SP, ADDR
### 0XDX
GB_RET_NC:
    andi $at, GB_F, C_FLAG
    bne $at, $zero, DECODE_NEXT # if C_FLAG != 0 skip return
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _GB_RET
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    nop
    nop
    nop
GB_POP_DE:
    addi ADDR, GB_SP, 0
    jal GB_DO_READ_16
    addi GB_SP, GB_SP, 2
    andi GB_SP, GB_SP, 0xFFFF
    srl GB_D, $v0, 8 # store B
    andi GB_E, $v0, 0xFF # store C
    j DECODE_NEXT
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
GB_JP_NC:
    andi $at, GB_F, C_FLAG
    bne $at, $zero, _SKIP_JP # if Z_FLAG != 0 skip jump
    nop
    jal GB_JP
    nop
    nop
    nop
    nop
GB_ERROR_0:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_CALL_NC:
    andi $at, GB_F, C_FLAG
    bne $at, $zero, _SKIP_JP # if Z_FLAG != 0 skip the call
    nop
    j _GB_CALL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 4 # update cycles run
    nop
    nop
    nop
GB_PUSH_DE:
    addi GB_SP, GB_SP, -2
    andi GB_SP, GB_SP, 0xFFFF
    addi ADDR, GB_SP, 0
    sll VAL, GB_D, 8
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 3 # update cycles run
    j GB_DO_WRITE_16
    or VAL, VAL, GB_E
    nop
GB_SUB_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _SUB_FROM_A
    addi Param0, $v0, 0
    nop
    nop
    nop
    nop
GB_RST_10H:
    addi GB_SP, GB_SP, -2
    andi ADDR, GB_SP, 0xFFFF
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    jal SET_GB_PC
    addi Param0, $zero, 0x0010
    j GB_DO_WRITE_16
    move GB_SP, ADDR
GB_RET_C:
    andi $at, GB_F, C_FLAG
    beq $at, $zero, DECODE_NEXT # if Z_FLAG == 0 skip RET
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _GB_RET
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    nop
    nop
    nop
GB_RETI:
    addi $at, $zero, INTERRUPTS_ENABLED
    jal CHECK_FOR_INTERRUPT
    sb $at, CPU_STATE_INTERRUPTS(CPUState)
    jal CALCULATE_NEXT_STOPPING_POINT
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    j _GB_RET
    nop
    nop
GB_JP_C:
    andi $at, GB_F, C_FLAG
    beq $at, $zero, _SKIP_JP # if Z_FLAG == 0 skip jump
    nop
    jal GB_JP
    nop
    nop
    nop
    nop
GB_ERROR_1:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_CALL_C:
    andi $at, GB_F, C_FLAG
    beq $at, $zero, _SKIP_JP # if Z_FLAG == 0 skip call
    nop
    j _GB_CALL
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 4 # update cycles run
    nop
    nop
    nop
GB_ERROR_2:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_SBC_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _SBC_FROM_A
    addi Param0, $v0, 0
    nop
    nop
    nop
    nop
GB_RST_18H:
    addi GB_SP, GB_SP, -2
    andi ADDR, GB_SP, 0xFFFF
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    jal SET_GB_PC
    addi Param0, $zero, 0x0018
    j GB_DO_WRITE_16
    move GB_SP, ADDR
### 0xEX
GB_LDH_a8_A:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    ori ADDR, $v0, 0xFF00
    j GB_DO_WRITE_REGISTERS
    move VAL, GB_A
    nop
    nop
    nop
GB_POP_HL:
    addi ADDR, GB_SP, 0
    jal GB_DO_READ_16
    addi GB_SP, GB_SP, 2
    andi GB_SP, GB_SP, 0xFFFF
    srl GB_H, $v0, 8 # store B
    andi GB_L, $v0, 0xFF # store C
    j DECODE_NEXT
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
GB_LDH_C_A:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    addi ADDR, GB_C, 0
    ori ADDR, ADDR, 0xFF00
    j GB_DO_WRITE_REGISTERS
    addi VAL, GB_A, 0
    nop
    nop
    nop
GB_ERROR_3:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_ERROR_4:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_PUSH_HL:
    addi GB_SP, GB_SP, -2
    andi GB_SP, GB_SP, 0xFFFF
    addi ADDR, GB_SP, 0
    sll VAL, GB_H, 8
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 3 # update cycles run
    j GB_DO_WRITE_16
    or VAL, VAL, GB_L
    nop
GB_AND_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    and GB_A, GB_A, $v0
    bne GB_A, $zero, DECODE_NEXT
    ori GB_F, $zero, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
GB_RST_20H:
    addi GB_SP, GB_SP, -2
    andi ADDR, GB_SP, 0xFFFF
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    jal SET_GB_PC
    addi Param0, $zero, 0x0020
    j GB_DO_WRITE_16
    move GB_SP, ADDR
GB_ADD_SP_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 3 # update cycles run
    sll $v0, $v0, 24 #sign extend
    j _ADD_TO_SP
    sra Param0, $v0, 24
    nop
    nop
    nop
GB_JP_HL:
    sll Param0, GB_H, 8
    jal SET_GB_PC
    or Param0, Param0, GB_L
    j DECODE_NEXT
    nop
    nop
    nop
    nop
GB_LD_a16_A:
    jal READ_NEXT_INSTRUCTION_16
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 3 # update cycles run
    addi ADDR, $v0, 0
    j GB_DO_WRITE
    addi VAL, GB_A, 0
    nop
    nop
    nop
GB_ERROR_5:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_ERROR_6:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_ERROR_7:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_XOR_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    xor GB_A, GB_A, $v0
    bne GB_A, $zero, DECODE_NEXT
    andi GB_F, $zero, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
GB_RST_28H:
    addi GB_SP, GB_SP, -2
    andi ADDR, GB_SP, 0xFFFF
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    jal SET_GB_PC
    addi Param0, $zero, 0x0028
    j GB_DO_WRITE_16
    move GB_SP, ADDR
### 0xFX
GB_LDH_A_a8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    jal GB_DO_READ_REGISTERS
    ori ADDR, $v0, 0xFF00
    j DECODE_NEXT
    move GB_A, $v0
    nop
    nop
GB_POP_AF:
    addi ADDR, GB_SP, 0
    jal GB_DO_READ_16
    addi GB_SP, GB_SP, 2
    andi GB_SP, GB_SP, 0xFFFF
    srl GB_A, $v0, 8 # store A
    andi GB_F, $v0, 0xF0 # store F
    j DECODE_NEXT
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
GB_LDH_A_C:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    jal GB_DO_READ_REGISTERS
    ori ADDR, GB_C, 0xFF00
    j DECODE_NEXT
    move GB_A, $v0
    nop
    nop
    nop
GB_DI:
    addi $at, $zero, 0
    j DECODE_NEXT
    sb $at, CPU_STATE_INTERRUPTS(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_ERROR_8:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_PUSH_AF:
    addi GB_SP, GB_SP, -2
    andi GB_SP, GB_SP, 0xFFFF
    addi ADDR, GB_SP, 0
    sll VAL, GB_A, 8
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 3 # update cycles run
    j GB_DO_WRITE_16
    or VAL, VAL, GB_F
    nop
GB_OR_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    or GB_A, GB_A, $v0
    bne GB_A, $zero, DECODE_NEXT
    ori GB_F, $zero, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
GB_RST_30H:
    addi GB_SP, GB_SP, -2
    andi ADDR, GB_SP, 0xFFFF
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    jal SET_GB_PC
    addi Param0, $zero, 0x0030
    j GB_DO_WRITE_16
    move GB_SP, ADDR
GB_LD_HL_SP_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    sll $v0, $v0, 24 #sign extend
    sra $v0, $v0, 24
    add GB_L, GB_SP, $v0
    j _MASK_HL
    srl GB_H, GB_L, 8
    nop
GB_LD_SP_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll GB_SP, GB_H, 8
    j DECODE_NEXT
    or GB_SP, GB_SP, GB_L
    nop
    nop
    nop
    nop
GB_LD_A_a16:
    jal READ_NEXT_INSTRUCTION_16
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 3 # update cycles run
    jal GB_DO_READ
    move ADDR, $v0
    move GB_A, $v0
    j DECODE_NEXT
    nop
    nop
GB_EI:
    addi $at, $zero, INTERRUPTS_ENABLED
    jal CHECK_FOR_INTERRUPT
    sb $at, CPU_STATE_INTERRUPTS(CPUState)
    jal CALCULATE_NEXT_STOPPING_POINT
    nop
    j DECODE_NEXT
    nop
    nop
GB_ERROR_9:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_ERROR_10:
    addi $at, $zero, STOP_REASON_ERROR
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_CP_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j _CP_A
    addi Param0, $v0, 0
    nop
    nop
    nop
    nop
GB_RST_38H:
    addi GB_SP, GB_SP, -2
    andi ADDR, GB_SP, 0xFFFF
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    jal SET_GB_PC
    addi Param0, $zero, 0x0038
    j GB_DO_WRITE_16
    move GB_SP, ADDR

######################
# Return instructions left to run
######################
GB_BREAK_LOOP:
    jal CALCULATE_DIV_VALUE
    sb GB_A, CPU_STATE_A(CPUState)
    jal CALCULATE_TIMA_VALUE
    sb GB_F, CPU_STATE_F(CPUState)

    sb GB_B, CPU_STATE_B(CPUState)
    sb GB_C, CPU_STATE_C(CPUState)

    sb GB_D, CPU_STATE_D(CPUState)
    sb GB_E, CPU_STATE_E(CPUState)
    sb GB_H, CPU_STATE_H(CPUState)
    sb GB_L, CPU_STATE_L(CPUState)

    sh GB_SP, CPU_STATE_SP(CPUState)
    sh GB_PC, CPU_STATE_PC(CPUState)
    
    # calculate the number of cycles run
    lw $v0, CPU_STATE_CYCLES_RUN(CPUState)
    sub $v0, CYCLES_RUN, $v0

    # We don't want CYCLES_RUN
    # to overflow while emulating the CPU
    # since CPU_STATE_NEXT_TIMER should always
    # be >= CYCLES_RUN
    # to prevent this, when exiting the
    # emulation we check if CYCLES_RUN is
    # above 0x80000000 if it is we 
    # decrement both CYCLES_RUN and
    # by that much CPU_STATE_NEXT_TIMER
    lui TMP2, 0x8000
    sltu $at, CYCLES_RUN, TMP2
    bnez $at, _GB_BREAK_LOOP_SAVE_CYCLES
    nop
    lw $at, CPU_STATE_NEXT_TIMER(CPUState)
    sub $at, $at, TMP2
    sw $at, CPU_STATE_NEXT_TIMER(CPUState)
    sub CYCLES_RUN, CYCLES_RUN, TMP2

_GB_BREAK_LOOP_SAVE_CYCLES:
    sw CYCLES_RUN, CPU_STATE_CYCLES_RUN(CPUState)
    
    lw $s0, ST_S0($sp) # restore caller registers
    lw $s1, ST_S1($sp) 
    lw $s2, ST_S2($sp) 
    lw $s3, ST_S3($sp) 
    lw $s4, ST_S4($sp) 
    lw $s5, ST_S5($sp) 
    lw $s6, ST_S6($sp) 
    lw $s7, ST_S7($sp) 
    lw $ra, ST_RA($sp) # load return address
    jr $ra
    addi $sp, $sp, STACK_SIZE
    
#######################
# Reads the byte at PC
# increments the program counter
# then returns the byte in register $v0
#######################

READ_NEXT_INSTRUCTION_16:
    addi $sp, $sp, -0x4
    sw $ra, 0($sp)
    jal READ_NEXT_INSTRUCTION
    nop
    jal READ_NEXT_INSTRUCTION
    move $v1, $v0
    sll $v0, $v0, 8
    or $v0, $v0, $v1
    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 0x4

READ_NEXT_INSTRUCTION:
    # read at PC, increment PC, return value
    add $at, GB_PC, PC_MEMORY_BANK
    lbu $v0, 0($at)
    addi Param0, GB_PC, 1
    andi Param0, Param0, 0xFFFF

    #intenionally continue into next label
SET_GB_PC:
    xor $at, GB_PC, Param0
    andi $at, $at, 0xF000
    beq $zero, $at, _SET_GB_PC_FINISH
    move GB_PC, Param0

    # if memory is within register memory
    ori $at, $zero, MM_REGISTER_START
    sltu $at, GB_PC, $at
    bnez $at, _SET_GB_PC_REGUALR_BANK 
    nop
    j _SET_GB_PC_UPDATE_BANK
    addi $at, Memory, MEMORY_MISC_START

_SET_GB_PC_REGUALR_BANK:
    srl $at, GB_PC, 12
    sll $at, $at, 2
    add $at, Memory, $at
    lw $at, 0($at)
    
_SET_GB_PC_UPDATE_BANK:
    andi Param0, GB_PC, 0xF000
    sub PC_MEMORY_BANK, $at, Param0
    
_SET_GB_PC_FINISH:
    jr $ra
    nop

########################
# Calculates when the next timer interrupt will happen
# Stomps on TMP2
# Automatically calls CALCULATE_NEXT_STOPPING_POINT
########################

CALCULATE_NEXT_TIMER_INTERRUPT:
    read_register_direct TMP2, REG_TAC # load the timer attributes table
    andi $at, TMP2, REG_TAC_STOP_BIT # check if interrupts are enabled
    beq $at, $zero, _CALCULATE_NEXT_TIMER_INTERRUPT_NONE # if timers are off, do nothing
    # input clock divider pattern is 0->256, 1->4, 2->16, 3->64
    # or (1 << (((dividerIndex - 1) & 0x3) + 1) * 2)
    addi TMP2, TMP2, -1 # 
    andi TMP2, TMP2, REG_TAC_CLOCK_SELECT
    addi TMP2, TMP2, 1
    sll TMP2, TMP2, 1
    # calculate the difference between the current time and
    # when the timer overflows
    read_register_direct $at, REG_TIMA
    sub $at, $zero, $at
    addi $at, $at, 0x100
    # shift the diffence by the clock divider
    sllv $at, $at, TMP2
    add $at, CYCLES_RUN, $at # make offset relative to cycles run
    j CALCULATE_NEXT_STOPPING_POINT
    # calculate the next interrupt time
    sw $at, CPU_STATE_NEXT_TIMER(CPUState)

_CALCULATE_NEXT_TIMER_INTERRUPT_NONE:
    la $at, 0xFFFFFFFF
    j CALCULATE_NEXT_STOPPING_POINT
    sw $at, CPU_STATE_NEXT_TIMER(CPUState)

########################
# Update DIV register to the correct value
# Stomps on TMP2
########################

CALCULATE_DIV_VALUE:
    # DIV = (((CYCLES_RUN << 2) + _REG_DIV_OFFSET) >> 8) & 0xFF
    sll $v0, CYCLES_RUN, 2
    read_register16_direct $at, _REG_DIV_OFFSET
    add $v0, $v0, $at
    srl $v0, $v0, 8
    jr $ra
    write_register_direct $v0, REG_DIV

########################
# Update TIMA register to the correct value
# Stomps on TMP2
########################

CALCULATE_TIMA_VALUE:
    lw TMP2, CPU_STATE_NEXT_TIMER(CPUState)
    addiu $v0, TMP2, 1 
    # if there is no timer running, do nothing
    beq $v0, $zero, _CALCULATE_TIMA_VALUE_NONE 
    
    read_register_direct $at, REG_TAC # load the timer attributes table
    # input clock divider pattern is 0->256, 1->4, 2->16, 3->64
    # or (1 << (((dividerIndex - 1) & 0x3) + 1) * 2)
    addi $at, $at, -1 # 
    andi $at, $at, REG_TAC_CLOCK_SELECT
    addi $at, $at, 1
    sll $at, $at, 1

    # calculate cycles until next interrupt
    # operands intentionally swapped to avoid needing
    # to negate the result later
    sub $v0, CYCLES_RUN, TMP2
    # shift the diffence by the clock divider
    srlv $v0, $v0, $at

    # write TIMA register
    write_register_direct $v0, REG_TIMA

_CALCULATE_TIMA_VALUE_NONE:
    jr $ra
    nop

########################
# Determines the next time a screen event happens
########################

# The following are typical when the display is enabled:
#   Mode 2  2_____2_____2_____2_____2_____2___________________2____
#   Mode 3  _33____33____33____33____33____33__________________3___
#   Mode 0  ___000___000___000___000___000___000________________000
#   Mode 1  ____________________________________11111111111111_____

# Mode 2    Mode 3      Mode 0
# 20        43          51  
# Mode 1 line
# 1140

CALCULATE_NEXT_SCREEN_INTERRUPT:
    read_register_direct $at, REG_LCDC
    andi $at, $at, REG_LCDC_LCD_ENABLE
    beqz $at, _CALCULATE_NEXT_SCREEN_INTERRUPT_OFF

    read_register_direct TMP2, REG_LCDC_STATUS
    slti $at, TMP2, 144
    beqz $at, _CALCULATE_NEXT_SCREEN_INTERRUPT_V_BLANK

    read_register_direct TMP2, REG_LCDC_STATUS
    andi TMP2, TMP2, REG_LCDC_STATUS_MODE

    li $at, REG_LCDC_STATUS_MODE_2
    beq TMP2, $at, _CALCULATE_NEXT_SCREEN_FINISH
    li $v0, REG_LCDC_STATUS_MODE_2_CYCLES
    
    li $at, REG_LCDC_STATUS_MODE_3
    beq TMP2, $at, _CALCULATE_NEXT_SCREEN_FINISH
    li $v0, REG_LCDC_STATUS_MODE_3_CYCLES

    j _CALCULATE_NEXT_SCREEN_FINISH
    li $v0, REG_LCDC_STATUS_MODE_0_CYCLES

_CALCULATE_NEXT_SCREEN_INTERRUPT_V_BLANK:
    li $v0, REG_LCDC_STATUS_MODE_1_CYCLES
_CALCULATE_NEXT_SCREEN_FINISH:
    # todo check for double speed mode
    add $v0, $v0, CYCLES_RUN
    jr $ra
    sw $v0, CPU_STATE_NEXT_SCREEN(CPUState)
    
_CALCULATE_NEXT_SCREEN_INTERRUPT_OFF:
    la $v0, ~0
    jr $ra
    sw $v0, CPU_STATE_NEXT_SCREEN(CPUState)

########################
# Determines the next time a special action
# Needs to occur which can be
#    Finished running cycles
#    Timer overflow
#    Interrupt
# Stomps on TMP2 and TMP3
########################

CALCULATE_NEXT_STOPPING_POINT:
    # if an interrupt has been requested
    # then then always stop
    lbu TMP2, CPU_STATE_NEXT_INTERRUPT(CPUState)
    bne TMP2, $zero, _CALCULATE_NEXT_STOPPING_POINT_FINISH
    li CycleTo, 0
    
    # deterime if CycleTo or NextTimer is smaller
    lw $at, CPU_STATE_NEXT_TIMER(CPUState)
    lw CycleTo, ST_CYCLE_TO($sp)
    sltu TMP2, CycleTo, $at
    bnez TMP2, _CALCULATE_NEXT_STOPPING_POINT_CHECK_SCREEN
    lw TMP2, CPU_STATE_NEXT_SCREEN(CPUState)
    move CycleTo, $at
    # compare current CycleTo to next screen event
_CALCULATE_NEXT_STOPPING_POINT_CHECK_SCREEN:
    sltu TMP3, TMP2, CycleTo
    beqz TMP3, _CALCULATE_NEXT_STOPPING_POINT_FINISH
    nop
    move CycleTo, TMP2
_CALCULATE_NEXT_STOPPING_POINT_FINISH:
    jr $ra
    nop


#############################
# Check if any interrupts have been requested
# And saves the requested interrupt CPU_STATE_NEXT_INTERRUPT
#############################

CHECK_FOR_INTERRUPT:
    # first check if interrupts are enabled
    lbu $at, CPU_STATE_INTERRUPTS(CPUState)
    beq $at, $zero, _CHECK_FOR_INTERRUPT_EXIT
    addi TMP2, $zero, 0

    # see if any individual interrupts have been triggered
    read_register_direct $at, REG_INTERRUPTS_REQUESTED
    read_register_direct TMP2, REG_INTERRUPTS_ENABLED
    and $at, TMP2, $at
    beq $at, $zero, _CHECK_FOR_INTERRUPT_EXIT
    addi TMP2, $zero, 0

    andi TMP2, $at, INTERRUPTS_V_BLANK
    bne TMP2, $zero, _CHECK_FOR_INTERRUPT_SAVE
    nop
    
    andi TMP2, $at, INTERRUPTS_LCDC
    bne TMP2, $zero, _CHECK_FOR_INTERRUPT_SAVE
    nop
    
    andi TMP2, $at, INTERRUPTS_TIMER
    bne TMP2, $zero, _CHECK_FOR_INTERRUPT_SAVE
    nop
    
    andi TMP2, $at, INTERRUPTS_SERIAL
    bne TMP2, $zero, _CHECK_FOR_INTERRUPT_SAVE
    nop
    
    andi TMP2, $at, INTERRUPTS_INPUT
    bne TMP2, $zero, _CHECK_FOR_INTERRUPT_SAVE
    nop
    
    j _CHECK_FOR_INTERRUPT_EXIT
    addi TMP2, $zero, 0

_CHECK_FOR_INTERRUPT_SAVE:
    ori CycleTo, $zero, 0 # run interrupt next cycle
_CHECK_FOR_INTERRUPT_EXIT:
    jr $ra
    sb TMP2, CPU_STATE_NEXT_INTERRUPT(CPUState)


########################
# Checks Param0 = REG_LCDC_STATUS
#        TMP3 = REG_LY
# 
########################

CHECK_LCDC_STAT_FLAG:
    # read the mode
    andi $at, Param0, REG_LCDC_STATUS_MODE
    li $v0, REG_LCDC_H_BLANK_INT
    sllv $at, $v0, $at # flag to check by mode
    move $v0, Param0
    andi $at, $v0, (REG_LCDC_H_BLANK_INT | REG_LCDC_V_BLANK_INT | REG_LCDC_OAM_INT)
    bnez $at, _CHECK_LCDC_STAT_FLAG_1
    andi $at, Param0, REG_LCDC_LYC_INT
    beqz $at, _CHECK_LCDC_STAT_FLAG_0
    nop
    # check if LYC == LY
    read_register_direct $v0, REG_LYC
    beq $v0, TMP3, _CHECK_LCDC_STAT_FLAG_1
    nop

    # check weird corner case where mode = 1 and ENABLE_OAM
    andi $at, Param0, REG_LCDC_OAM_INT
    beqz $at, _CHECK_LCDC_STAT_FLAG_0
    andi $at, Param0, REG_LCDC_STATUS_MODE
    addi $at, $at, -1
    beqz $at, _CHECK_LCDC_STAT_FLAG_1
    nop

_CHECK_LCDC_STAT_FLAG_0:
    jr $ra
    li $v0, 0

_CHECK_LCDC_STAT_FLAG_1:
    jr $ra
    li $v0, 1

########################
# Determines the action to take now that
# CycleTo has been reached
# Stomps on Param0
########################

HANDLE_STOPPING_POINT:
    # check for video interrupts first 
    lw $at, CPU_STATE_NEXT_SCREEN(CPUState)
    sltu TMP2, CYCLES_RUN, $at
    bnez $at, _HANDLE_STOPPING_POINT_CHECK_TIMER
    nop
    
########################

_HANDLE_STOPPING_POINT_SCREEN:
    read_register_direct TMP3, REG_LY
    # load current LCDC status flag
    jal CHECK_LCDC_STAT_FLAG
    read_register_direct Param0, REG_LCDC_STATUS

    # check if current in V_BLANK
    slti $at, TMP3, GB_SCREEN_H
    beqz $at, _HANDLE_STOPPING_POINT_SCREEN_V_BLANK
    andi Param0, Param0, %lo(~REG_LCDC_STATUS_LYC) # clear LYC flag

    andi $at, Param0, REG_LCDC_STATUS_MODE
    # if current mode is 0 check for screen wrap
    beqz $at, _HANDLE_STOPPING_POINT_SCREEN_ROW_WRAP
    addi Param0, Param0, 1 #increment mode

    # clear LYC flag again in case previous mode was 0x3
    andi Param0, Param0, %lo(~REG_LCDC_STATUS_LYC)

    j _HANDLE_STOPPING_POINT_SCREEN_FINISH
    nop
_HANDLE_STOPPING_POINT_SCREEN_ROW_WRAP:
    addi TMP3, TMP3, 1 # increment LY
    li $at, GB_SCREEN_H
    bne $at, TMP3, _HANDLE_STOPPING_POINT_SCREEN_FINISH
    addi Param0, Param0, 1 # set Param0 to 2 for wrap to next row
    
    # request v blank interrupt
    read_register_direct TMP2, REG_INTERRUPTS_REQUESTED
    ori TMP2, TMP2, INTERRUPT_V_BLANK
    write_register_direct TMP2, REG_INTERRUPTS_REQUESTED

    j _HANDLE_STOPPING_POINT_SCREEN_FINISH
    addi Param0, Param0, -1 # put mode back to mode 1
_HANDLE_STOPPING_POINT_SCREEN_V_BLANK:
    addi TMP3, TMP3, 1
    slti $at, TMP3, GB_SCREEN_LINES
    bnez $at, _HANDLE_STOPPING_POINT_SCREEN_FINISH
    nop
    # wrap back around to top of LCD
    addi Param0, Param0, 1 # mode 2
    move TMP3, $zero   # set LY to 0

    # if the cpu should only run until the next frame
    # then update CYCLE_TO to be CYCLES_RUN
    lbu $at, CPU_STATE_RUN_UNTIL_FRAME(CPUState)
    beqz $at, _HANDLE_STOPPING_POINT_SCREEN_FINISH
    move $at, CYCLES_RUN
    sw $at, ST_CYCLE_TO($sp)
_HANDLE_STOPPING_POINT_SCREEN_FINISH:
    # save new LY
    write_register_direct TMP3, REG_LY
    # check if LYC flag should be set
    read_register_direct $at, REG_LCY

    bne $at, TMP3, _HANDLE_STOPPING_POINT_SCREEN_SKIP_LYC
    nop
    ori Param0, Param0, REG_LCDC_STATUS_LYC
_HANDLE_STOPPING_POINT_SCREEN_SKIP_LYC:
    # save new STATUS
    write_register_direct Param0, REG_LCDC_STATUS

    # check if LCDC_STAT interrupt flag should be set
    # $v0 holds the previous STAT_FLAG
    jal CHECK_LCDC_STAT_FLAG # calculate current STAT_FLAG
    move TMP2, $v0 # store previous valg to TMP2
    slt $at, TMP2, $v0 # if previous stat < current state then trigger interrupt
    beqz $at, _HANDLE_STOPPING_POINT_SCREEN_SKIP_INT
    nop
    # request interrupt
    read_register_direct TMP2, REG_INTERRUPTS_REQUESTED
    ori TMP2, TMP2, INTERRUPT_LCD_STAT
    write_register_direct TMP2, REG_INTERRUPTS_REQUESTED

    jal CHECK_FOR_INTERRUPT
    nop

_HANDLE_STOPPING_POINT_SCREEN_SKIP_INT:
    jal CALCULATE_NEXT_SCREEN_INTERRUPT
    nop
    jal CALCULATE_NEXT_STOPPING_POINT
    nop
    j DECODE_NEXT
    nop
    
########################

_HANDLE_STOPPING_POINT_CHECK_TIMER:
    # check timer second
    lw $at, CPU_STATE_NEXT_TIMER(CPUState)
    sltu TMP2, CYCLES_RUN, $at
    bne TMP2, $zero, _HANDLE_STOPPING_POINT_CHECK_INTERRUPT
    nop
    read_register_direct TMP2, REG_TMA
    write_register_direct TMP2, REG_TIMA

    read_register_direct TMP2, REG_INTERRUPTS_REQUESTED
    ori TMP2, TMP2, INTERRUPTS_TIMER
    jal CHECK_FOR_INTERRUPT
    write_register_direct TMP2, REG_INTERRUPTS_REQUESTED

    jal CALCULATE_NEXT_TIMER_INTERRUPT
    nop

    j DECODE_NEXT
    nop
    
########################

_HANDLE_STOPPING_POINT_CHECK_INTERRUPT:
    lbu $at, CPU_STATE_NEXT_INTERRUPT(CPUState)
    beq $at, $zero, _HANDLE_STOPPING_POINT_BREAK

    ori ADDR, $zero, 0x40 # load the base address for interrupt jumps
    andi $at, $at, 0x1F # mask bits
    srl TMP2, $at, 1 # calculte which bit to jump tp
_HANDLE_STOPPING_POINT_INT_JUMP_LOOP:
    beq TMP2, $zero, _HANDLE_STOPPING_POINT_CLEAR_INTERRUPT
    srl TMP2, TMP2, 1
    j _HANDLE_STOPPING_POINT_INT_JUMP_LOOP
    addi ADDR, ADDR, 0x8

_HANDLE_STOPPING_POINT_CLEAR_INTERRUPT:
    # clear requested interrupt
    xori $at, $at, 0xFF
    read_register_direct TMP2, REG_INTERRUPTS_REQUESTED
    and TMP2, TMP2, $at
    write_register_direct TMP2, REG_INTERRUPTS_REQUESTED

    sb $zero, CPU_STATE_NEXT_INTERRUPT(CPUState) # clear pending interrupt
    sb $zero, CPU_STATE_INTERRUPTS(CPUState) # disable interrupts
    
    jal CALCULATE_NEXT_STOPPING_POINT
    nop
    
    addi GB_SP, GB_SP, -2 # reserve space in stack
    andi GB_SP, GB_SP, 0xFFFF
    move VAL, GB_PC # set current PC to be saved
    jal SET_GB_PC
    move Param0, ADDR # set the new PC
    j GB_DO_WRITE_16
    move ADDR, GB_SP # set the write address
    
########################

_HANDLE_STOPPING_POINT_BREAK:
    j GB_BREAK_LOOP
    nop

######################
# Checks to see if a timer interrupt would wake up the CPU
######################

GB_SIMULATE_HALTED:
    # todo timer values
    lw $at, ST_CYCLE_TO($sp)
    j GB_BREAK_LOOP
    move CYCLES_RUN, $at # catch cpu clock up to requested cycles to run

#######################
# Reads the address HL into $v0
#######################

READ_HL:
    sll ADDR, GB_H, 8 # load high order bits
    j GB_DO_READ
    or ADDR, ADDR, GB_L # load low order bits

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

.include "asm/math.s"
.include "asm/cpu_inst_prefix.s"
.include "asm/memory.s"
