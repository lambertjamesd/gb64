
########################
# Calculates when the next timer interrupt will happen
# Stomps on TMP2
########################

CALCULATE_NEXT_TIMER_INTERRUPT:
    addi $sp, $sp, -4
    sw $ra, 0($sp)
    read_register_direct TMP2, REG_TAC # load the timer attributes table
    andi $at, TMP2, REG_TAC_STOP_BIT # check if interrupts are enabled
    beqz $at, _CALCULATE_NEXT_TIMER_INTERRUPT_NONE # if timers are off, do nothing
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
    # calculate the next interrupt time
    sw $at, CPU_STATE_NEXT_TIMER(CPUState)
    sll TMP2, $at, 8
    jal QUEUE_STOPPING_POINT
    addi TMP2, TMP2, CPU_STOPPING_POINT_TYPE_TIMER
    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 4
    nop
_CALCULATE_NEXT_TIMER_INTERRUPT_NONE:
    la $at, 0xFFFFFFFF
    sw $at, CPU_STATE_NEXT_TIMER(CPUState)
    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 4

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
    andi $v0, $v0, 0xFF
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
    beqz $v0, _CALCULATE_TIMA_VALUE_NONE 
    nop

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
    andi $v0, $v0, 0xFF # mask the result

    # write TIMA register
    write_register_direct $v0, REG_TIMA

_CALCULATE_TIMA_VALUE_NONE:
    jr $ra
    nop
    
#############################

CHECK_FOR_INTERRUPT:
    read_register_direct TMP2, REG_INTERRUPTS_REQUESTED
    read_register_direct $at, REG_INTERRUPTS_ENABLED
    and $at, $at, TMP2
    beqz $at, _CHECK_FOR_INTERRUPT_FINISH
    nop
    sll TMP2, CYCLES_RUN, 8
    j QUEUE_STOPPING_POINT 
    addi TMP2, TMP2, CPU_STOPPING_POINT_TYPE_INTERRUPT
_CHECK_FOR_INTERRUPT_FINISH:
    jr $ra
    nop

#############################
# Sets the interrupt request flag and wakes up cpu if this is a new interrupt
# VAL is the requested interrupt
#############################

REQUEST_INTERRUPT:
    read_register_direct TMP2, REG_INTERRUPTS_REQUESTED
    and $at, TMP2, VAL
    bnez $at, _REQUEST_INTERRUPT_FINISH # check if the interrupt was already requested
    or TMP2, TMP2, VAL
    write_register_direct TMP2, REG_INTERRUPTS_REQUESTED

    read_register_direct $at, REG_INTERRUPTS_ENABLED
    and $at, $at, VAL
    beqz $at, _REQUEST_INTERRUPT_FINISH
    nop
    sb $zero, CPU_STATE_STOP_REASON(CPUState)
    sll TMP2, CYCLES_RUN, 8
    j QUEUE_STOPPING_POINT 
    addi TMP2, TMP2, CPU_STOPPING_POINT_TYPE_INTERRUPT
_REQUEST_INTERRUPT_FINISH:
    jr $ra
    nop

########################
# Check to unhalt the CPUState
########################

# TODO handle halt bug
CHECK_FOR_UNHALT:
    lbu $at, CPU_STATE_INTERRUPTS(CPUState)
    bnez $at, DECODE_NEXT
    nop
    read_register_direct $at, REG_INTERRUPTS_REQUESTED
    read_register_direct TMP2, REG_INTERRUPTS_ENABLED
    and $at, $at, TMP2
    andi $at, $at, 0x1F
    bnez $at, CHECK_FOR_UNHALT_DO_IT
    nop
    j DECODE_NEXT
    nop
CHECK_FOR_UNHALT_DO_IT:
    j DECODE_NEXT
    nop
    sb $zero, CPU_STATE_STOP_REASON(CPUState) # wake up from stop/halt

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
# loads next stopping point
# and handles the current one
########################

DEQUEUE_STOPPING_POINT:
    addi $sp, $sp, -4
    sw $ra, 0($sp)

    lw $at, CPU_STATE_NEXT_STOPPING_POINT(CPUState)
    addi $at, $at, CPU_STATE_STOPPING_POINT_SIZE # move to next stopping point
    sw $at, CPU_STATE_NEXT_STOPPING_POINT(CPUState)
    add TMP2, $at, CPUState

    lw $at, (CPU_STATE_STOPPING_POINTS)(TMP2)
    srl CycleTo, $at, 8 # save new stopping point
    
    # load previous stopping reason
    lw $at, (CPU_STATE_STOPPING_POINTS - CPU_STATE_STOPPING_POINT_SIZE)(TMP2)
    sw $zero, (CPU_STATE_STOPPING_POINTS - CPU_STATE_STOPPING_POINT_SIZE)(TMP2) # clear next stopping point
    andi $at, $at, 0xFF # mask the stopping point type

    sltiu TMP2, $at, CPU_STOPPING_POINT_TYPE_EXIT
    beqz TMP2, HANDLE_DEQUEUE_EXIT

    sll $at, $at, 2 # align jump table to 4 bytes
    la TMP2, DEQUEUE_STOPPING_POINT_J_TABLE
    add $at, TMP2, $at
    lw $at, 0($at)
    jr $at
    nop
.data
DEQUEUE_STOPPING_POINT_J_TABLE: .word HANDLE_DEQUEUE_EXIT, ENTER_MODE_0, ENTER_MODE_1, ENTER_MODE_2, ENTER_MODE_3, HANDLE_DEQUEUE_TIMER, HANDLE_DEQUEUE_INTERRUPT, HANDLE_DEQUEUE_EXIT
.text

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

ENTER_MODE_0:
    read_register_direct TMP3, REG_LY
    # load current LCDC status flag
    jal CHECK_LCDC_STAT_FLAG
    read_register_direct Param0, REG_LCDC_STATUS
    andi Param0, Param0, %lo(~REG_LCDC_STATUS_MODE)

    addi TMP2, CYCLES_RUN, REG_LCDC_STATUS_MODE_0_CYCLES
    sll TMP2, TMP2, 8
    li TMP4, CPU_STOPPING_POINT_TYPE_SCREEN_2
    slti $at, TMP3, GB_SCREEN_H
    bnez $at, _ENTER_MODE_0_NEXT_MODE
    nop
    li TMP4, CPU_STOPPING_POINT_TYPE_SCREEN_1
_ENTER_MODE_0_NEXT_MODE:
    jal QUEUE_STOPPING_POINT
    add TMP2, TMP2, TMP4
    j CHECK_LSTAT_INTERRUPT
    nop

ENTER_MODE_1:
    read_register_direct TMP3, REG_LY
    # load current LCDC status flag
    jal CHECK_LCDC_STAT_FLAG
    read_register_direct Param0, REG_LCDC_STATUS
    andi Param0, Param0, %lo(~REG_LCDC_STATUS_MODE)
    addi Param0, Param0, 1
    addi TMP2, CYCLES_RUN, REG_LCDC_STATUS_MODE_1_CYCLES
    sll TMP2, TMP2, 8
    slti $at, TMP3, GB_SCREEN_LINES - 1
    li TMP4, CPU_STOPPING_POINT_TYPE_SCREEN_1
    bnez $at, _ENTER_MODE_1_NEXT_MODE
    addi TMP3, TMP3, 1
    li TMP4, CPU_STOPPING_POINT_TYPE_SCREEN_2
_ENTER_MODE_1_NEXT_MODE:
    jal QUEUE_STOPPING_POINT
    add TMP2, TMP2, TMP4
    j CHECK_LSTAT_INTERRUPT
    write_register_direct TMP3, REG_LY
    
ENTER_MODE_2:
    read_register_direct TMP3, REG_LY
    # load current LCDC status flag
    jal CHECK_LCDC_STAT_FLAG
    read_register_direct Param0, REG_LCDC_STATUS
    andi Param0, Param0, %lo(~REG_LCDC_STATUS_MODE)
    addi Param0, Param0, 2
    addi TMP2, CYCLES_RUN, REG_LCDC_STATUS_MODE_2_CYCLES
    sll TMP2, TMP2, 8
    jal QUEUE_STOPPING_POINT
    addi TMP2, TMP2, CPU_STOPPING_POINT_TYPE_SCREEN_3
    addi TMP3, TMP3, 1
    li $at, GB_SCREEN_LINES
    bne $at, TMP3, _ENTER_MODE_2_SKIP_SCREEN_START
    nop
    lbu $at, CPU_STATE_RUN_UNTIL_FRAME(CPUState)
    beqz $at, _ENTER_MODE_2_SKIP_SCREEN_START
    li TMP3, 0
    jal REMOVE_STOPPING_POINT
    li Param0, CPU_STOPPING_POINT_TYPE_EXIT
    sll TMP2, CYCLES_RUN, 8
    jal QUEUE_STOPPING_POINT
    addi TMP2, TMP2, CPU_STOPPING_POINT_TYPE_EXIT
_ENTER_MODE_2_SKIP_SCREEN_START:
    j CHECK_LSTAT_INTERRUPT
    write_register_direct TMP3, REG_LY

ENTER_MODE_3:
    read_register_direct TMP3, REG_LY
    # load current LCDC status flag
    jal CHECK_LCDC_STAT_FLAG
    read_register_direct Param0, REG_LCDC_STATUS
    andi Param0, Param0, %lo(~REG_LCDC_STATUS_MODE)
    addi Param0, Param0, 3
    addi TMP2, CYCLES_RUN, REG_LCDC_STATUS_MODE_3_CYCLES
    sll TMP2, TMP2, 8
    jal QUEUE_STOPPING_POINT
    addi TMP2, TMP2, CPU_STOPPING_POINT_TYPE_SCREEN_0
    j CHECK_LSTAT_INTERRUPT
    nop

CHECK_LSTAT_INTERRUPT:
    write_register_direct Param0, REG_LCDC_STATUS
    jal CHECK_LCDC_STAT_FLAG
    move TMP2, $v0
    slt $at, TMP2, $v0 # if previous stat < current state then trigger interrupt
    beqz $at, CHECK_LSTAT_INTERRUPT_SKIP_INTERRUPT
    nop
    # request interrupt
    jal REQUEST_INTERRUPT
    li VAL, INTERRUPT_LCD_STAT
CHECK_LSTAT_INTERRUPT_SKIP_INTERRUPT:
    j _FINISH_DEQUEUE_INTERRUPT
    nop

HANDLE_DEQUEUE_TIMER:
    read_register_direct TMP2, REG_TMA
    write_register_direct TMP2, REG_TIMA

    jal REQUEST_INTERRUPT
    li VAL, INTERRUPTS_TIMER

    jal CALCULATE_NEXT_TIMER_INTERRUPT
    nop

    j _FINISH_DEQUEUE_INTERRUPT
    nop

HANDLE_DEQUEUE_INTERRUPT:
    # first check if interrupts are enabled
    lbu $at, CPU_STATE_INTERRUPTS(CPUState)
    beqz $at, _FINISH_DEQUEUE_INTERRUPT
    nop
    # see if any individual interrupts have been triggered
    read_register_direct $at, REG_INTERRUPTS_REQUESTED
    read_register_direct TMP2, REG_INTERRUPTS_ENABLED
    and $at, TMP2, $at
    beqz $at, _FINISH_DEQUEUE_INTERRUPT
    nop

    andi TMP2, $at, INTERRUPTS_V_BLANK
    bnez TMP2, _HANDLE_DEQUEUE_INTERRUPT_SAVE
    li ADDR, 0x40 # load the base address for interrupt jumps
    
    andi TMP2, $at, INTERRUPTS_LCDC
    bnez TMP2, _HANDLE_DEQUEUE_INTERRUPT_SAVE
    li ADDR, 0x48 # load the base address for interrupt jumps
    
    andi TMP2, $at, INTERRUPTS_TIMER
    bnez TMP2, _HANDLE_DEQUEUE_INTERRUPT_SAVE
    li ADDR, 0x50 # load the base address for interrupt jumps
    
    andi TMP2, $at, INTERRUPTS_SERIAL
    bnez TMP2, _HANDLE_DEQUEUE_INTERRUPT_SAVE
    li ADDR, 0x58 # load the base address for interrupt jumps
    
    andi TMP2, $at, INTERRUPTS_INPUT
    bnez TMP2, _HANDLE_DEQUEUE_INTERRUPT_SAVE
    li ADDR, 0x60 # load the base address for interrupt jumps
    
    j _FINISH_DEQUEUE_INTERRUPT
    nop

_HANDLE_DEQUEUE_INTERRUPT_SAVE:
    # clear requested interrupt
    xori $at, TMP2, 0xFF
    read_register_direct TMP2, REG_INTERRUPTS_REQUESTED
    and TMP2, TMP2, $at
    write_register_direct TMP2, REG_INTERRUPTS_REQUESTED

    sb $zero, CPU_STATE_INTERRUPTS(CPUState) # disable interrupts
    sb $zero, CPU_STATE_STOP_REASON(CPUState) # wake up from stop/halt
    
    addi GB_SP, GB_SP, -2 # reserve space in stack
    andi GB_SP, GB_SP, 0xFFFF
    move VAL, GB_PC # set current PC to be saved
    jal SET_GB_PC
    move Param0, ADDR # set the new PC
    jal GB_DO_WRITE_16_CALL
    move ADDR, GB_SP # set the write address
    j _FINISH_DEQUEUE_INTERRUPT
    nop
    
HANDLE_DEQUEUE_EXIT:
    j GB_BREAK_LOOP
    addi $sp, $sp, 4

_FINISH_DEQUEUE_INTERRUPT:
    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 4


########################
# $a2 is CycleTo and since it is 
# updated at the end of the function
# it is used as a temporary variable
########################

QUEUE_STOPPING_POINT:
    addi $sp, $sp, -4
    sw TMP3, 0($sp)
    lw $a2, CPU_STATE_NEXT_STOPPING_POINT(CPUState)
    add $a2, $a2, CPUState
_QUEUE_STOPPING_POINT_CHECK_NEXT:
    # check if at end of loop
    addi $at, CPUState, CPU_STATE_STOPPING_POINT_SIZE * CPU_STATE_STOPPING_POINT_MAX_COUNT
    beq $at, $a2, _QUEUE_STOPPING_POINT_SET
    nop
    # check if found insertion point
    lw TMP3, CPU_STATE_STOPPING_POINTS($a2)
    sltu $at, TMP2, TMP3
    bnez $at, _QUEUE_STOPPING_POINT_SET
    nop
    sw TMP3, (CPU_STATE_STOPPING_POINTS-CPU_STATE_STOPPING_POINT_SIZE)($a2) # move current element back one slot
    j _QUEUE_STOPPING_POINT_CHECK_NEXT
    addi $a2, $a2, CPU_STATE_STOPPING_POINT_SIZE
_QUEUE_STOPPING_POINT_SET:
    sw TMP2, (CPU_STATE_STOPPING_POINTS-CPU_STATE_STOPPING_POINT_SIZE)($a2) # save new stopping point
    lw $a2, CPU_STATE_NEXT_STOPPING_POINT(CPUState) # 
    addi $a2, $a2, -CPU_STATE_STOPPING_POINT_SIZE
    sw $a2, CPU_STATE_NEXT_STOPPING_POINT(CPUState)
    add $a2, $a2, CPUState
    
    lw CycleTo, CPU_STATE_STOPPING_POINTS($a2)
    srl CycleTo, CycleTo, 8 # timer is stored in the upper 24 bits

    lw TMP3, 0($sp)
    jr $ra
    addi $sp, $sp, 4

READ_NEXT_STOPPING_POINT:
    lw CycleTo, CPU_STATE_NEXT_STOPPING_POINT(CPUState)
    add CycleTo, CycleTo, CPUState
    lw CycleTo, CPU_STATE_STOPPING_POINTS(CPUState)
    jr $ra
    srl CycleTo, CycleTo, 8 # timer is stored in the upper 24 bits

#######################

REMOVE_STOPPING_POINT:
    # load read and write head
    addi TMP2, CPUState, CPU_STATE_STOPPING_POINTS + (CPU_STATE_STOPPING_POINT_MAX_COUNT - 1) * CPU_STATE_STOPPING_POINT_SIZE
    addi TMP3, CPUState, CPU_STATE_STOPPING_POINTS + (CPU_STATE_STOPPING_POINT_MAX_COUNT - 1) * CPU_STATE_STOPPING_POINT_SIZE
_REMOVE_STOPPING_POINT_NEXT:
    addi $at, CPUState, CPU_STATE_STOPPING_POINTS - CPU_STATE_STOPPING_POINT_SIZE
    # check if end has been reached
    beq $at, TMP3, _REMOVE_STOPPING_POINT_END
    lw $at, 0(TMP3)
    # check if empty stopping point has been reached
    beqz $at, _REMOVE_STOPPING_POINT_END
    andi $at, $at, 0xFF
    # check if this element should be removed
    beq $at, Param0, _REMOVE_STOPPING_POINT_SKIP
    nop
_REMOVE_STOPPING_POINT_KEEP:
    lw $at, 0(TMP3)
    sw $at, 0(TMP2)
    addi TMP3, TMP3, -CPU_STATE_STOPPING_POINT_SIZE
    j _REMOVE_STOPPING_POINT_NEXT
    addi TMP2, TMP2, -CPU_STATE_STOPPING_POINT_SIZE
_REMOVE_STOPPING_POINT_SKIP:
    j _REMOVE_STOPPING_POINT_NEXT
    addi TMP3, TMP3, -CPU_STATE_STOPPING_POINT_SIZE
_REMOVE_STOPPING_POINT_END:
    sub $at, TMP2, CPUState
    addi $at, $at, CPU_STATE_STOPPING_POINT_SIZE-CPU_STATE_STOPPING_POINTS
    sw $at, CPU_STATE_NEXT_STOPPING_POINT(CPUState)
_REMOVE_STOPPING_POINT_ZERO_LOOP:
    beq TMP2, TMP3, _REMOVE_STOPPING_POINT_EXIT
    addi TMP2, TMP2, -CPU_STATE_STOPPING_POINT_SIZE 
    j _REMOVE_STOPPING_POINT_ZERO_LOOP
    sw $zero, CPU_STATE_STOPPING_POINT_SIZE(TMP2)
_REMOVE_STOPPING_POINT_EXIT:
    jr $ra
    nop
