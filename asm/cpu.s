
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

.set VALIDATE_STATE, 0

.data
.include "asm/_cpu_inst_table.s"
.text

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

DECODE_NEXT:
.if VALIDATE_STATE
    nop
    jal CHECK_FOR_INVALID_STATE
    nop
.endif
    # check if an an event has been reached
    sltu $at, CYCLES_RUN, CycleTo
    beqz $at, DECODE_STOPPING_POINT
    # get the next instruction to decode
    lbu $v0, 0(PC_MEM_POINTER)
    addi PC_MEM_POINTER, PC_MEM_POINTER, 1
    addi GB_PC, GB_PC, 1
DECODE_V0:
    lui $at, %hi(cpuInstructionTable)
    sll $v0, $v0, 2 # multiply instruction value by 4
    add $at, $v0, $at
    lw TMP2, (%lo(cpuInstructionTable))($at)
    jr TMP2
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR

DECODE_STOPPING_POINT:
    jal DEQUEUE_STOPPING_POINT
    nop
    j DECODE_NEXT
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
    lbu $at, 0(PC_MEM_POINTER)
    lbu $v0, 1(PC_MEM_POINTER)
    sll $v0, $v0, 8
    or $v0, $v0, $at
    addi PC_MEM_POINTER, PC_MEM_POINTER, 2
    jr $ra
    addi GB_PC, GB_PC, 2

READ_NEXT_INSTRUCTION:
    # read at PC, increment PC, return value
    lbu $v0, 0(PC_MEM_POINTER)
    addi PC_MEM_POINTER, PC_MEM_POINTER, 1
    jr $ra
    addi GB_PC, GB_PC, 1

SET_GB_PC:
    andi GB_PC, Param0, 0xFFFF
    andi $at, GB_PC, 0xF000
    xori $at, $at, 0xF000
    bnez $at, _SET_GB_PC_REGULAR_BANK

    # check if PC jumped between 0xF000-0xFDFF and 0xFF80-0xFFFE
    li $at, MM_REGISTER_START
    sltu $at, GB_PC, $at
    beqz $at, _SET_GB_PC_MISC_MEM
    nop
    # echo memory bank
    j _SET_GB_PC_OFFSET_POINTER
    lw PC_MEM_POINTER, (MEMORY_ADDR_TABLE + 4 * 0xD)(Memory)
_SET_GB_PC_MISC_MEM:
    # 0xE00 since memory is indexed relative to 0xF000 and not 0xFE00
    j _SET_GB_PC_OFFSET_POINTER
    addi PC_MEM_POINTER, Memory, (MEMORY_MISC_START - 0xE00)

_SET_GB_PC_REGULAR_BANK:
    srl $at, GB_PC, 12 # git top 4 bits
    sll $at, $at, 2 # multiply by 4
    add $at, Memory, $at # access relative to memory map
    lw PC_MEM_POINTER, 0($at) # load bank pointer
    
_SET_GB_PC_OFFSET_POINTER:
    andi TMP2, GB_PC, 0xFFF
    jr $ra
    add PC_MEM_POINTER, PC_MEM_POINTER, TMP2

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
.include "asm/_registers.s"
.include "asm/_mbc7.s"
.include "asm/_mbc.s"
