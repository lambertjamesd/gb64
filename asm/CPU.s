
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

.set CPUState, $a0
.set MemoryStart, $a1
.set InstructionCount, $a2

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
.eqv CPU_STATE_STOPPED, 0xC

.eqv Z_FLAG, 0x80
.eqv N_FLAG, 0x40
.eqv H_FLAG, 0x20
.eqv C_FLAG, 0x10

.eqv STACK_SIZE, 0x4
.eqv ST_RA, 0x0

.macro clear_flags flags
    andi GB_F, GB_F, %lo(flags)
.endm

.macro set_flags flags
    ori GB_F, GB_F, flags
.endm

.global runZ80CPU 
.balign 4 
runZ80CPU:
    addi $sp, $sp, -STACK_SIZE
    sw $ra, ST_RA($sp)

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

    j GB_FINISH_RUN
    nop
    # saveReturnAddress
    # load registers from CPUState

DECODE_NEXT:
    jal READ_NEXT_INSTRUCTION # get the next instruction to decode
    # la $at, GB_NOP
    lui $at, %hi(GB_NOP)
    ori $at, $at, %lo(GB_NOP)
    add $ra, $at, $v0
    jr $ra
    nop
GB_NOP: # start of jump table
    j DECODE_NEXT
    nop
GB_LD_BC_D16:
    jal READ_NEXT_INSTRUCTION # read immedate values
    nop
    jal READ_NEXT_INSTRUCTION
    addi GB_B, $v0, 0 # store B
    j DECODE_NEXT
    addi GB_C, $v0, 0 # store C
GB_LD_BC_A:
    add VAL, GB_A, 0 # write the value to store
    sll ADDR, GB_B, 8 # write upper address
    j GB_DO_WRITE # call store subroutine
    ori ADDR, GB_C, 0 # write address to store
GB_INC_BC:
    addi GB_C, GB_C, 1 # incement the register
    srl $at, GB_C, 8
    andi GB_C, GB_C, 0xFF # keep at 8 bits
    add GB_B, GB_B, $at # add carry bit
    j DECODE_NEXT
    andi GB_B, GB_B, 0xFF # keep at 8 bits
GB_INC_B:
    jal GB_INC # call increment
    addi Param0, GB_B, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_B, Param0, 0 # move register back from call parameter
GB_DEC_B:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_B, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_B, Param0, 0 # move register back from call parameter
GB_LD_B_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    nop
    j DECODE_NEXT
    addi GB_B, $v0, 0 #store value
GB_RLCA:
    jal GB_RLC_IMPL # do RLC
    addi Param0, GB_A, 0 # store A into param
    j DECODE_NEXT
    addi GB_A, Param0, 0 # store result back into A
GB_LD_A16_SP:
    jal READ_NEXT_INSTRUCTION # read immediate value
    nop
    jal READ_NEXT_INSTRUCTION # read immediate value
    sll GB_SP, $v0 #store upper address
    j DECODE_NEXT
    ori GB_SP, $v0, 0 #store lower address
GB_ADD_HL_BC:
    add $at, GB_H, GB_B # add upper order bits
    sll $at, $at, 8 # shift upper order bits
    add $at, $at, GB_L # add lower order bits
    add $at, $at, GB_C 
    andi GB_L, $at, 0xFF # store lower order bits
    srl GB_H, $at, 8 # store higher order bits
    j DECODE_NEXT
    andi GB_H, GB_H, 0xFF # keep higher order bits at 8 bits
GB_LD_A_BC:
    sll ADDR, GB_B, 8 # load upper address
    jal GB_DO_READ # call read instruction
    ori ADDR, GB_C, 0 # load lower address
    j DECODE_NEXT
    addi GB_A, $v0, 0 # store result into a
GB_DEC_BC:
    addi GB_C, GB_C, -1
    sra $at, GB_C, 8
    j DECODE_NEXT
    add GB_B, GB_B, $at
GB_INC_C:
    jal GB_INC # call increment
    addi Param0, GB_C, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_C, Param0, 0 # move register back from call parameter
GB_DEC_C:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_C, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_C, Param0, 0 # move register back from call parameter
GB_LD_C_D8:
    jal READ_NEXT_INSTRUCTION # read immediate value
    nop
    j DECODE_NEXT
    addi GB_C, $v0, 0 #store value
GB_RRCA:
    jal GB_RRC_IMPL # call rotate
    addi Param0, GB_A, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_A, Param0, 0 # store paramter back
GB_STOP:
    addi $at, $zero, 1
    sw $at, CPU_STATE_STOPPED(CPUState)
    j DECODE_NEXT
    addi InstructionCount, $zero, 0

######################
# Return instructions left to run
######################
GB_FINISH_RUN:
    lw $ra, ST_RA($sp)
    addi $sp, $sp, STACK_SIZE
    jr $ra
    addi $v0, GB_PC, 0

######################
# Writes VAL to ADDR
######################

GB_DO_WRITE:
    lui $at, 0xFFFF
    # first check if address is in shadow ram (0xE000-0xFDFF)
    addi $at, $at, 0x2000 
    add $at, ADDR, $at # ADDR - 0xE000
    bltz $at, _GB_DO_WRITE
    addi $at, $at, -0X1E00 # ADDR - 0xFE00
    bgtz $at, _GB_DO_WRITE
    nop
    addi ADDR, ADDR, -0x2000 # shift the address into working ram
_GB_DO_WRITE:
    add ADDR, ADDR, MemoryStart # use address relative to emulator memory
    j DECODE_NEXT
    sb VAL, 0(ADDR) # store the byte
    
######################
# Reads ADDR into $v0
######################

GB_DO_READ:
    lui $at, 0xFFFF
    # first check if address is in shadow ram (0xE000-0xFDFF)
    addi $at, $at, 0x2000 
    add $at, ADDR, $at # ADDR - 0xE000
    bltz $at, _GB_DO_WRITE
    addi $at, $at, -0X1E00 # ADDR - 0xFE00
    bgtz $at, _GB_DO_WRITE
    nop
    addi ADDR, ADDR, -0x2000 # shift the address into working ram
_GB_DO_READ:
    add ADDR, ADDR, MemoryStart # use address relative to emulator memory
    jr $ra
    lbu $v0, 0(ADDR)

#######################
# Increments the high byte of Param0
# and sets any associated flags
#######################
GB_INC:
    addi Param0, Param0, 0x1
    andi Param0, Param0, 0xFF # keep register at 8 bits
    bne Param0, $zero, _GB_INC_CHECK_HALF
    clear_flags ~N_FLAG
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
    clear_flags ~H_FLAG # clear H flag

#######################
# Decrements the high byte of Param0
# and sets any associated flags
#######################
GB_DEC:
    addi Param0, Param0, 0xFF
    andi Param0, Param0, 0xFF # keep register at 8 bits
    bne Param0, $zero, _GB_DEC_CHECK_HALF
    set_flags N_FLAG # set N flag

    clear_flags ~H_FLAG # clear H flag
    jr $ra
    set_flags Z_FLAG # set Z flag

_GB_DEC_CHECK_HALF:
    andi $at, Param0, 0x3 # check if h flag should be set
    addi $at, $at, -0x3
    bne $at, $zero, _GB_DEC_DONE
    nop
    jr $ra
    clear_flags ~H_FLAG # clear H flag

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
    sll $at, $at, 4 # shift carry bit into C_FLAG position
    andi GB_F, GB_F, 0x0 # clear flags
    set_flags $at # set C_FLAG
    jr $ra
    andi Param0, Param0, 0xFF # keep param 8 bits

_GB_RLC_IMPL_IS_ZERO:
    andi GB_F, GB_F, 0x0 # clear all flags
    jr $ra
    set_flags Z_FLAG # set Z_FLAG

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

    andi $at, $at, 0x1 # mask carry bit
    sll $at, $at, 4 # shift carry bit into C_FLAG position
    andi GB_F, GB_F, 0x0 # clear flags
    jr $ra
    set_flags $at # set C_FLAG

_GB_RRC_IMPL_IS_ZERO:
    andi GB_F, GB_F, 0x0 # clear all flags
    jr $ra
    set_flags Z_FLAG # set Z_FLAG

#######################
# Reads the byte at PC
# increments the program counter
# then returns the byte in register $v0
#######################

READ_NEXT_INSTRUCTION:
    # read at PC, increment PC, return value
    add $v0, GB_PC, MemoryStart
    lbu $v0, 0($v0)
    jr $ra
    addi GB_PC, GB_PC, 1
