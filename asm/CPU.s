
.set noat      # allow manual use of $at
.set noreorder # don't insert nops after branches

#######################
# A0 CPUState
# A1 RAMPointer
# A2 InstructionCount
#
#######################

.set GB_A, $s0
.set GB_F, $s1
.set GB_B, $s2
.set GB_C, $s3
.set GB_D, $s4
.set GB_E, $s5
.set GB_H, $s6
.set GB_L, $s7
.set GB_SP, $t0
.set GB_PC, $t1

.set ADDR, $t2
.set VAL,  $t3
.set TMP2,  $t4

.set CPUState, $a0
.set Memory, $a1
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
.eqv CPU_STATE_INTERRUPTS, 0xD

.eqv MEMORY_ADDR_TABLE, 0x00
.eqv MEMORY_REGISTER_TABLE, 0x40
.eqv MEMORY_BANK_SWITCHING, 0x60
.eqv MEMORY_MISC_START, 0x64

.eqv MM_REGISTER_START, 0xFE00

.eqv Z_FLAG, 0x80
.eqv N_FLAG, 0x40
.eqv H_FLAG, 0x20
.eqv C_FLAG, 0x10

.eqv STACK_SIZE, 0x28
.eqv ST_RA, 0x0
.eqv ST_CYCLES_REQUESTED, 0x4
.eqv ST_S0, 0x8
.eqv ST_S1, 0xC
.eqv ST_S2, 0x10
.eqv ST_S3, 0x14
.eqv ST_S4, 0x18
.eqv ST_S5, 0x1C
.eqv ST_S6, 0x20
.eqv ST_S7, 0x24

.eqv SAVE_TMP_SIZE, 0x28

.eqv CYCLES_PER_INSTR, 0x1

.eqv STOP_REASON_NONE, 0x0
.eqv STOP_REASON_STOP, 0x1
.eqv STOP_REASON_HALT, 0x2
.eqv STOP_REASON_INTERRUPT_RET, 0x3
.eqv STOP_REASON_ERROR, 0x4

.eqv INTERRUPTS_V_BLANK, 0x01;
.eqv INTERRUPTS_LCDC, 0x02;
.eqv INTERRUPTS_TIMER, 0x04;
.eqv INTERRUPTS_SERIAL, 0x08;
.eqv INTERRUPTS_INPUT, 0x10;
.eqv INTERRUPTS_ENABLED, 0x80;

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
    addi GB_C, $v0, 0 # store C
    j DECODE_NEXT
    addi GB_B, $v0, 0 # store B
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
    sll Param0, $v0, 8 #store upper address
    or ADDR, Param0, $v0 #store lower address
    j GB_DO_WRITE_16
    addi VAL, GB_SP, 0 # store value to write
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
    addi GB_E, $v0, 0 # store E
    j DECODE_NEXT
    addi GB_D, $v0, 0 # store D
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
_SKIP_JR:
    addi GB_PC, GB_PC, 1
    j DECODE_NEXT
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
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
    andi $at, GB_F, Z_FLAG # check z flag
    beq $at, $zero, _SKIP_JR # skip jump if not zero
    nop
    j GB_JR
    nop
    nop
    nop
    nop
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
    andi $at, GB_F, C_FLAG # check z flag
    bne $at, $zero, _SKIP_JR # skip jump if not zero
    nop
    j GB_JR
    nop
    nop
    nop
    nop
GB_LD_SP_D16:
    jal READ_NEXT_INSTRUCTION # read immedate values
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    jal READ_NEXT_INSTRUCTION
    addi GB_SP, $v0, 0 # store low bits
    sll $v0, $v0, 8
    j DECODE_NEXT
    or GB_SP, GB_SP, $v0 # store high bits
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
    andi $at, GB_F, C_FLAG # check z flag
    beq $at, $zero, _SKIP_JR # skip jump if not zero
    nop
    j GB_JR
    nop
    nop
    nop
    nop
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_B, 0
    nop
    nop
    nop
GB_LD_HL_C:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_C, 0
    nop
    nop
    nop
GB_LD_HL_D:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_D, 0
    nop
    nop
    nop
GB_LD_HL_E:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_E, 0
    nop
    nop
    nop
GB_LD_HL_H:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_H, 0
    nop
    nop
    nop
GB_LD_HL_L:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    j GB_DO_WRITE # call read instruction
    addi VAL, GB_L, 0
    nop
    nop
    nop
GB_LD_HALT:
    addi $at, $zero, STOP_REASON_HALT
    j GB_BREAK_LOOP # exit early
    sb $at, CPU_STATE_STOP_REASON(CPUState)
    nop
    nop
    nop
    nop
    nop
GB_LD_HL_A:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    j DECODE_NEXT
    addi GB_PC, GB_PC, 2
    nop
    nop
    nop
### 0xCX
GB_RET_NZ:
    andi $at, GB_F, Z_FLAG
    bne $at, $zero, DECODE_NEXT # if Z_FLAG != 0 skip return
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    j _GB_RET
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    nop
    nop
    nop
GB_POP_BC:
    addi ADDR, GB_SP, 0
    jal GB_DO_READ_16
    addi GB_SP, GB_SP, 2
    srl GB_B, $v0, 8 # store B
    andi GB_C, $v0, 0xFF # store C
    j DECODE_NEXT
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    nop
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
    addi ADDR, GB_PC, 0
    jal GB_DO_READ_16
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    j DECODE_NEXT
    addi GB_PC, $v0, 0
    nop
    nop
    nop
GB_CALL_NZ:
    andi $at, GB_F, Z_FLAG
    bne $at, $zero, _SKIP_JP # if Z_FLAG != 0 skip the call
    nop
    jal GB_CALL
    nop
    nop
    nop
    nop
GB_PUSH_BC:
    addi GB_SP, GB_SP, -2
    addi ADDR, GB_SP, 0
    sll VAL, GB_B, 8
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 3 # update cycles run
    j GB_DO_WRITE_16
    or VAL, VAL, GB_C
    nop
    nop
GB_ADD_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    j _ADD_TO_A
    addi Param0, $v0, 0
    nop
    nop
    nop
    nop
GB_RST_00H:
    addi GB_SP, GB_SP, -2
    addi ADDR, GB_SP, 0
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    j GB_DO_WRITE_16
    addi GB_PC, $zero, 0x0000
    nop
    nop
GB_RET_Z:
    andi $at, GB_F, Z_FLAG
    beq $at, $zero, DECODE_NEXT # if Z_FLAG == 0 skip RET
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    j _GB_RET
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    nop
    nop
    nop
GB_RET:
    j _GB_RET
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
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
    jal GB_CALL
    nop
    nop
    nop
    nop
GB_CALL:
    jal GB_DO_READ_16
    addi ADDR, GB_PC, 0
    addi GB_SP, GB_SP, -2
    addi ADDR, GB_SP, 0
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 4 # update cycles run
    addi VAL, GB_PC, 2
    j GB_DO_WRITE_16
    addi GB_PC, $v0, 0
GB_ADC_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    j _ADC_TO_A
    addi Param0, $v0, 0
    nop
    nop
    nop
    nop
GB_RST_08H:
    addi GB_SP, GB_SP, -2
    addi ADDR, GB_SP, 0
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    j GB_DO_WRITE_16
    addi GB_PC, $zero, 0x0008
    nop
    nop
### 0XDX
GB_RET_NC:
    andi $at, GB_F, C_FLAG
    bne $at, $zero, DECODE_NEXT # if C_FLAG != 0 skip return
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    j _GB_RET
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    nop
    nop
    nop
GB_POP_DE:
    addi ADDR, GB_SP, 0
    jal GB_DO_READ_16
    addi GB_SP, GB_SP, 2
    srl GB_D, $v0, 8 # store B
    andi GB_E, $v0, 0xFF # store C
    j DECODE_NEXT
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    nop
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
    jal GB_CALL
    nop
    nop
    nop
    nop
GB_PUSH_DE:
    addi GB_SP, GB_SP, -2
    addi ADDR, GB_SP, 0
    sll VAL, GB_D, 8
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 3 # update cycles run
    j GB_DO_WRITE_16
    or VAL, VAL, GB_E
    nop
    nop
GB_SUB_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    j _SUB_FROM_A
    addi Param0, $v0, 0
    nop
    nop
    nop
    nop
GB_RST_10H:
    addi GB_SP, GB_SP, -2
    addi ADDR, GB_SP, 0
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    j GB_DO_WRITE_16
    addi GB_PC, $zero, 0x0010
    nop
    nop
GB_RET_C:
    andi $at, GB_F, C_FLAG
    beq $at, $zero, DECODE_NEXT # if Z_FLAG == 0 skip RET
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    j _GB_RET
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    nop
    nop
    nop
GB_RETI:
    lbu $at, CPU_STATE_INTERRUPTS(CPUState)
    andi $at, $at, INTERRUPTS_ENABLED
    sb $at, CPU_STATE_INTERRUPTS(CPUState)
    j _GB_RET
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    nop
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
    jal GB_CALL
    nop
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    j _SBC_FROM_A
    addi Param0, $v0, 0
    nop
    nop
    nop
    nop
GB_RST_18H:
    addi GB_SP, GB_SP, -2
    addi ADDR, GB_SP, 0
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    j GB_DO_WRITE_16
    addi GB_PC, $zero, 0x0018
    nop
    nop
### 0xEX
GB_LDH_a8_A:
    jal READ_NEXT_INSTRUCTION
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    ori ADDR, $v0, 0xFF00
    j GB_DO_WRITE_REGISTERS
    addi VAL, GB_A, 0
    nop
    nop
    nop
GB_POP_HL:
    addi ADDR, GB_SP, 0
    jal GB_DO_READ_16
    addi GB_SP, GB_SP, 2
    srl GB_H, $v0, 8 # store B
    andi GB_L, $v0, 0xFF # store C
    j DECODE_NEXT
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    nop
GB_LDH_C_A:
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    addi ADDR, GB_C, 0
    ori ADDR, $v0, 0xFF00
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
    addi ADDR, GB_SP, 0
    sll VAL, GB_H, 8
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 3 # update cycles run
    j GB_DO_WRITE_16
    or VAL, VAL, GB_L
    nop
    nop
GB_AND_A_d8:
    jal READ_NEXT_INSTRUCTION
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    and GB_A, GB_A, $v0
    bne GB_A, $zero, DECODE_NEXT
    andi GB_F, $zero, H_FLAG
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
GB_RST_20H:
    addi GB_SP, GB_SP, -2
    addi ADDR, GB_SP, 0
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    j GB_DO_WRITE_16
    addi GB_PC, $zero, 0x0020
    nop
    nop
GB_ADD_SP_d8:
    jal READ_NEXT_INSTRUCTION
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 3 # update cycles run
    srl $v0, $v0, 24 #sign extend
    j _ADD_TO_SP
    sra Param0, $v0, 24
    nop
    nop
    nop
GB_JP_HL:
    sll GB_PC, GB_H, 8
    j DECODE_NEXT
    or GB_PC, GB_L, 8
    nop
    nop
    nop
    nop
    nop
GB_LD_a16_A:
    addi ADDR, GB_PC, 0
    jal GB_DO_READ_16
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 3 # update cycles run
    addi GB_PC, GB_PC, 2
    addi ADDR, $v0, 0
    j GB_DO_WRITE
    addi VAL, GB_A, 0
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
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR # update cycles run
    xor GB_A, GB_A, $v0
    bne GB_A, $zero, DECODE_NEXT
    andi GB_F, $zero, 0
    j DECODE_NEXT
    set_flags Z_FLAG
    nop
GB_RST_28H:
    addi GB_SP, GB_SP, -2
    addi ADDR, GB_SP, 0
    addi CycleCount, CycleCount, -CYCLES_PER_INSTR * 2 # update cycles run
    addi VAL, GB_PC, 0
    j GB_DO_WRITE_16
    addi GB_PC, $zero, 0x0028
    nop
    nop
### 0xFX
    


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

######################
# Writes 16 bit VAL to ADDR
######################

GB_DO_WRITE_16:
    addiu $at, $zero, 0x8000
    sub $at, ADDR, $at
    bgez $at, _GB_DO_WRITE_16 # if ADDR >= 0x8000 just write
    nop 
    # todo do bank switching
    j DECODE_NEXT
    nop
_GB_DO_WRITE_16:
    srl $at, ADDR, 12 # load bank in $at
    andi ADDR, ADDR, 0xFFF # keep offset in ADDR
    sll $at, $at, 2 # word align the memory map offset
    add $at, $at, Memory # lookup start of bank in array at Memory
    lw $at, 0($at) # load start of memory bank
    add ADDR, ADDR, $at # use address relative to memory bank
    sb VAL, 0(ADDR) # store low byte
    srl VAL, VAL, 8 # shift high byte
    j DECODE_NEXT
    sb VAL, 1(ADDR) # store high byte


######################
# Writes VAL to ADDR
######################

GB_DO_WRITE:
    ori $at, $zero, MM_REGISTER_START
    sub $at, ADDR, $at 
    bgez $at, GB_DO_WRITE_REGISTERS # if ADDR >= 0xFE00 do register logic

    addiu $at, $zero, 0x8000
    sub $at, ADDR, $at
    bgez $at, _GB_DO_WRITE # if ADDR >= 0x8000 just write
    nop 
    j _GB_CALL_WRITE_CALLBACK # call bank switching callback
    sw $at, MEMORY_BANK_SWITCHING(CPUState)
_GB_DO_WRITE:
    srl $at, ADDR, 12 # load bank in $at
    andi ADDR, ADDR, 0xFFF # keep offset in ADDR
    sll $at, $at, 2 # word align the memory map offset
    add $at, $at, Memory # lookup start of bank in array at Memory
    lw $at, 0($at) # load start of memory bank
    add ADDR, ADDR, $at # use address relative to memory bank
    j DECODE_NEXT
    sb VAL, 0(ADDR) # store the byte
    
######################
# Writes VAL to ADDR in the range 0xFE00-0xFFFF
######################

GB_DO_WRITE_REGISTERS:
    li $at, -0xFF00
    add $at, $at, ADDR # ADDR relative to MISC memory
    bltz $at, _GB_BASIC_REGISTER_WRITE # just write the sprite memory bank
    addi $at, $at, -0x80
    bgez $at, _GB_BASIC_REGISTER_WRITE # just write memory above interrupt table

    addi $at, $at, 0x80 # move $at back into range 0x00 - 0x80
    srl $at, $at, 2 # get the upper nibble and multiply it by 4
    andi $at, $at, 0x3C # mask upper nibble to get jump offset
    add $at, $at, Memory # move jump relative to memory
    addi $at, $at, MEMORY_REGISTER_TABLE # move jump relative to register jump table
_GB_CALL_WRITE_CALLBACK: # $at points to the funcation to call
    addi $sp, $sp, -0x24 # save temporary registers
    sw $t0, 0x00($sp)
    sw $t1, 0x04($sp)
    sw $t2, 0x08($sp)
    sw $t3, 0x0C($sp)
    sw $t4, 0x10($sp)
    sw $a0, 0x14($sp)
    sw $a1, 0x18($sp)
    sw $a2, 0x1C($sp)
    sw $a3, 0x20($sp)

    move $a0, Memory
    move $a1, ADDR
    jalr $ra, $at # call register handler
    addi $a2, VAL, 0

    lw $t0, 0x00($sp) # restore temporary registers
    lw $t1, 0x04($sp)
    lw $t2, 0x08($sp)
    lw $t3, 0x0C($sp)
    lw $t4, 0x10($sp)
    lw $a0, 0x14($sp)
    lw $a1, 0x18($sp)
    lw $a2, 0x1C($sp)
    lw $a3, 0x20($sp)
    j DECODE_NEXT
    addi $sp, $sp, 0x24

_GB_BASIC_REGISTER_WRITE:
    li $at, MEMORY_MISC_START-MM_REGISTER_START
    add ADDR, $at, ADDR # ADDR relative to MISC memory
    add ADDR, Memory, ADDR # Relative to memory
    j DECODE_NEXT
    sb VAL, 0(ADDR)
    
######################
# Reads ADDR into $v0
######################

GB_DO_READ_16:
    srl $at, ADDR, 12 # load bank in $at
    andi ADDR, ADDR, 0xFFF # keep offset in ADDR
    sll $at, $at, 2 # word align the memory map offset
    add $at, $at, Memory # lookup start of bank in array at Memory
    lw $at, 0($at) # load start of memory bank
    add ADDR, ADDR, $at # use address relative to memory bank
    lbu $v0, 1(ADDR) # load high byte
    sll $v0, $v0, 8
    lbu $at, 0(ADDR) # load low byte
    jr $ra
    or $v0, $v0, $at # move low byte
    
######################
# Reads ADDR into $v0
######################

GB_DO_READ:
    ori $at, $zero, MM_REGISTER_START
    sub $at, ADDR, $at
    bgez $at, GB_DO_READ_REGISTERS # if ADDR >= 0xFE00

    srl $at, ADDR, 12 # load bank in $at
    andi ADDR, ADDR, 0xFFF # keep offset in ADDR
    sll $at, $at, 2 # word align the memory map offset
    add $at, $at, Memory # lookup start of bank in array at Memory
    lw $at, 0($at) # load start of memory bank
    add ADDR, ADDR, $at # use address relative to memory bank
    jr $ra
    lbu $v0, 0(ADDR) # load the byte

######################
# Reads the ADDR Value in the range 0xFE00-0xFFFF
######################

GB_DO_READ_REGISTERS:
    li $at, MEMORY_MISC_START-MM_REGISTER_START
    add ADDR, $at, ADDR # ADDR relative to MISC memory
    add ADDR, Memory, ADDR # Relative to memory
    jr $ra
    sb $v0, 0(ADDR)

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


#######################
# Reads the address HL into $v0
#######################

READ_HL:
    sll ADDR, GB_H, 8 # load high order bits
    j GB_DO_READ
    or ADDR, ADDR, GB_L # load low order bits

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
    sll $at, GB_F, 4                        # shift carry bit into position
    or Param0, Param0, $at                  # set carry bit
    sll $at, Param0, 4                      # shift new carry bit into pos
    andi GB_F, $at, 0x10                    # set carry bit

    srl Param0, Param0, 1                   # shift the bit once
    beq Param0, $zero, _GB_RR_IMPL_IS_ZERO
    andi Param0, Param0, 0xFF               # set to 8 bits
    jr $ra
    nop
_GB_RR_IMPL_IS_ZERO:
    jr $ra
    set_flags Z_FLAG                        # set Z_FLAG

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
# Pop top of stack and return to it
#######################

_GB_RET:
    addi ADDR, GB_SP, 0
    jal GB_DO_READ_16
    addi GB_SP, GB_SP, 2

    # TODO check for interrupt return

    j DECODE_NEXT
    addi GB_PC, $v0, 0
    
#######################
# Subtract Param0 to A
#######################

_GB_JP:
    jal READ_NEXT_INSTRUCTION
    nop
    jal READ_NEXT_INSTRUCTION
    sll TMP2, $v0, 8
    j DECODE_NEXT
    or GB_PC, TMP2, $v0

#######################
# Decimal encodes Param0
#######################

GB_DA:
    andi $at, Param0, 0x0F
    jr $ra
    nop
