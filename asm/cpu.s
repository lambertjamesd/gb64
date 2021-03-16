
.set noat      # allow manual use of $at
.set noreorder # don't insert nops after branches

.include "asm/registers.inc"
.include "asm/memory.inc"

.include "asm/_debug.s"

.macro clear_flags flags
    andi GB_F, GB_F, %lo(~(\flags))
.endm

.macro set_flags flags
    ori GB_F, GB_F, \flags
.endm

.set DEBUG_OUT, 0
.set VALIDATE_STATE, 0

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
    sw $fp, ST_FP($sp)
    sw $a3, ST_FLAGS($sp)

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

    move $fp, $sp

    jal SET_GB_PC
    # make sure GB_PC doesn't match Param0 to force bank load
    xori GB_PC, Param0, 0xFFFF 

    # load timer
    lw CYCLES_RUN, CPU_STATE_CYCLES_RUN(CPUState)
    sw CYCLES_RUN, ST_STARTING_CLOCKS($fp)

    read_register_direct $at, REG_KEY1
    andi $at, $at, REG_KEY1_CURRENT_SPEED
    beqz $at, _CALC_CYCLE_TO
    nop
    # double speed mode
    sll CycleTo, CycleTo, 1
_CALC_CYCLE_TO:
    add TMP2, CycleTo, CYCLES_RUN    # calculate upper bound of execution
    sll TMP2, TMP2, 8
    jal QUEUE_STOPPING_POINT
    addi TMP2, TMP2, CPU_STOPPING_POINT_TYPE_EXIT

    lbu $at, CPU_STATE_STOP_REASON(CPUState)
    bnez $at, GB_SIMULATE_HALTED
    nop

.if DEBUG_OUT
    la $at, 0x80700000 - 4
    la TMP4, 0x80700000
    sw TMP4, 0($at)
.endif

DECODE_NEXT:
.if VALIDATE_STATE
    nop
    jal CHECK_FOR_INVALID_STATE
    nop
.endif

    sltu $at, CYCLES_RUN, CycleTo
    bnez $at, _DECODE_NEXT_READ
    nop
    jal DEQUEUE_STOPPING_POINT
    nop
    j DECODE_NEXT
    nop
_DECODE_NEXT_READ:
    jal READ_NEXT_INSTRUCTION # get the next instruction to decode
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR

.if DEBUG_OUT
DEBUG_START:
    la $at, 0x80700000 - 4
    lw TMP4, 0($at)
    sw Memory, -4($at)
    sb $v0, 0(TMP4)
    sh GB_PC, 2(TMP4)
    addi TMP4, TMP4, 4

    la $at, 0x80800000
    sltu $at, TMP4, $at
    bne $at, $zero, _DEBUG_SKIP
    nop
    la TMP4, 0x80700000

_DEBUG_SKIP:
    la $at, 0x80700000 - 4
    sw TMP4, 0($at)
.endif

DECODE_V0:
    la $at, GB_NOP # load start of jump table
    sll $v0, $v0, 5 # multiply address by 32 (4 bytes * 8 instructions)
    add $ra, $at, $v0
    jr $ra
    nop
    #################
    # Each entry in the jump table needs
    # to be 8 instructions apart
    # any under must be padded with nops
.include "asm/_cpu_inst_0.s"
.include "asm/_cpu_inst_1.s"
.include "asm/_cpu_inst_2.s"
.include "asm/_cpu_inst_3.s"
.include "asm/_cpu_inst_4.s"
.include "asm/_cpu_inst_5.s"
.include "asm/_cpu_inst_6.s"
.include "asm/_cpu_inst_7.s"
.include "asm/_cpu_inst_8.s"
.include "asm/_cpu_inst_9.s"
.include "asm/_cpu_inst_A.s"
.include "asm/_cpu_inst_B.s"
.include "asm/_cpu_inst_C.s"
.include "asm/_cpu_inst_D.s"
.include "asm/_cpu_inst_E.s"
.include "asm/_cpu_inst_F.s"

######################
# Return instructions left to run
######################
GB_BREAK_LOOP:
    jal CALCULATE_DIV_VALUE
    sb GB_A, CPU_STATE_A(CPUState)
    jal CALCULATE_TIMA_VALUE
    sb GB_F, CPU_STATE_F(CPUState)

    jal GB_CALC_UNSCALED_CLOCKS
    sb GB_B, CPU_STATE_B(CPUState)
    sw $v0, CPU_STATE_UNSCALED_CYCLES_RUN(CPUState)
    sb GB_C, CPU_STATE_C(CPUState)

    sb GB_D, CPU_STATE_D(CPUState)
    sb GB_E, CPU_STATE_E(CPUState)
    sb GB_H, CPU_STATE_H(CPUState)
    sb GB_L, CPU_STATE_L(CPUState)

    sh GB_SP, CPU_STATE_SP(CPUState)
    sh GB_PC, CPU_STATE_PC(CPUState)

    # calculate the number of cycles run
    lw $v0, ST_STARTING_CLOCKS($fp)
    sub $v0, CYCLES_RUN, $v0
    
    read_register_direct $at, REG_KEY1
    andi $at, $at, REG_KEY1_CURRENT_SPEED
    beqz $at, _GB_BREAK_LOOP_SAVE_CYCLES
    nop
    # double speed mode
    srl $v0, $v0, 1
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
    lw $fp, ST_FP($sp)
    jr $ra
    addi $sp, $sp, STACK_SIZE
    
#######################
# Reads the byte at PC
# increments the program counter
# then returns the byte in register $v0
#######################

READ_NEXT_INSTRUCTION_16:
    addi $sp, $sp, -8
    sw $ra, 0($sp)
    jal READ_NEXT_INSTRUCTION
    nop
    jal READ_NEXT_INSTRUCTION
    move $v1, $v0
    sll $v0, $v0, 8
    or $v0, $v0, $v1
    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 8

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
    bnez $at, _SET_GB_PC_CHANGE_BANK
    nop
    # check if PC jumped between 0xF000-0xFDFF and 0xFF80-0xFFFE
    li $at, MEMORY_MISC_START
    sltu TMP2, GB_PC, $at
    sltu $at, Param0, $at
    beq TMP2, $at, _SET_GB_PC_FINISH
    nop
_SET_GB_PC_CHANGE_BANK:
    # if memory is within register memory
    la $at, MM_REGISTER_START
    sltu $at, Param0, $at
    bnez $at, _SET_GB_PC_REGULAR_BANK 
    nop
    j _SET_GB_PC_UPDATE_BANK
    # 0xE00 since memory is indexed relative to 0xF000 and not 0xFE00
    addi $at, Memory, (MEMORY_MISC_START - 0xE00) 

_SET_GB_PC_REGULAR_BANK:
    srl $at, Param0, 12 # git top 4 bits
    sll $at, $at, 2 # multiply by 4
    add $at, Memory, $at # access relative to memory map
    lw $at, 0($at) # load bank pointer
    
_SET_GB_PC_UPDATE_BANK:
    andi TMP2, Param0, 0xF000
    sub PC_MEMORY_BANK, $at, TMP2
    
_SET_GB_PC_FINISH:
    jr $ra
    move GB_PC, Param0

######################
# Checks to see if a timer interrupt would wake up the CPU
######################

GB_SIMULATE_HALTED:
    jal DEQUEUE_STOPPING_POINT # handle the next stopping point
    move CYCLES_RUN, CycleTo # update the clock
    lbu $at, CPU_STATE_STOP_REASON(CPUState) # check if CPU was woken up by an interrupt
    beqz $at, DECODE_NEXT
    nop
    j GB_SIMULATE_HALTED # loop, DEQUEUE_STOPPING_POINT will break the loop once it has been simulated
    nop

GB_CALC_UNSCALED_CLOCKS:
    lw $at, ST_STARTING_CLOCKS($fp)
    read_register_direct $v0, REG_KEY1
    andi $v0, $v0, REG_KEY1_CURRENT_SPEED
    beqz $v0, _GB_CALC_UNSCALED_CLOCKS_NORMAL_SPEED
    sub $at, CYCLES_RUN, $at
    srl $at, $at, 1
_GB_CALC_UNSCALED_CLOCKS_NORMAL_SPEED:
    lw $v0, CPU_STATE_UNSCALED_CYCLES_RUN(CPUState)
    jr $ra
    add $v0, $v0, $at

.include "asm/_stopping_point.s"
.include "asm/_branch.s"
.include "asm/_math.s"
.include "asm/_cpu_inst_prefix.s"
.include "asm/_memory.s"
.include "asm/_mbc.s"
