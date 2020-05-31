
########################
# Calculates when the next timer interrupt will happen
# Stomps on TMP2
# Automatically calls CALCULATE_NEXT_STOPPING_POINT
########################

CALCULATE_NEXT_TIMER_INTERRUPT:
    read_register_direct TMP2, REG_TAC # load the timer attributes table
    andi $at, TMP2, REG_TAC_STOP_BIT # check if interrupts are enabled
    beq $at, $zero, _CALCULATE_NEXT_TIMER_INTERRUPT_NONE # if timers are off, do nothing
    lbu $at, CPU_STATE_STOP_REASON(CPUState) # if the cpu is stopped, then stop the timer too
    bnez $at, _CALCULATE_NEXT_TIMER_INTERRUPT_NONE
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
    #determine if interrupt time is smallest
    lw $at, CPU_STATE_NEXT_INTERRUPT_TIME(CPUState)
    lw CycleTo, ST_CYCLE_TO($fp)
    sltu TMP2, CycleTo, $at
    bnez TMP2, _CALCULATE_NEXT_STOPPING_POINT_CHECK_TIMER
    lw TMP2, CPU_STATE_NEXT_TIMER(CPUState)
    move CycleTo, $at
_CALCULATE_NEXT_STOPPING_POINT_CHECK_TIMER:
    # deterime if CycleTo or NextTimer is smaller
    sltu $at, CycleTo, TMP2
    bnez $at, _CALCULATE_NEXT_STOPPING_POINT_CHECK_SCREEN
    lw $at, CPU_STATE_NEXT_SCREEN(CPUState)
    move CycleTo, TMP2
    # compare current CycleTo to next screen event
_CALCULATE_NEXT_STOPPING_POINT_CHECK_SCREEN:
    sltu TMP2, CycleTo, $at
    bnez TMP2, _CALCULATE_NEXT_STOPPING_POINT_FINISH
    nop
    move CycleTo, $at
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
    beqz $at, _CHECK_FOR_INTERRUPT_EXIT
    li TMP2, 0
    # check if an interrupt was already requested
    lbu $at, CPU_STATE_NEXT_INTERRUPT(CPUState)
    bnez $at, _CHECK_FOR_INTERRUPT_EXIT
    nop

    # default to no next interrupt
    la TMP3, ~0
    # see if any individual interrupts have been triggered
    read_register_direct $at, REG_INTERRUPTS_REQUESTED
    read_register_direct TMP2, REG_INTERRUPTS_ENABLED
    and $at, TMP2, $at
    beqz $at, _CHECK_FOR_INTERRUPT_EXIT
    li TMP2, 0

    andi TMP2, $at, INTERRUPTS_V_BLANK
    bnez TMP2, _CHECK_FOR_INTERRUPT_SAVE
    nop
    
    andi TMP2, $at, INTERRUPTS_LCDC
    bnez TMP2, _CHECK_FOR_INTERRUPT_SAVE
    nop
    
    andi TMP2, $at, INTERRUPTS_TIMER
    bnez TMP2, _CHECK_FOR_INTERRUPT_SAVE
    nop
    
    andi TMP2, $at, INTERRUPTS_SERIAL
    bnez TMP2, _CHECK_FOR_INTERRUPT_SAVE
    nop
    
    andi TMP2, $at, INTERRUPTS_INPUT
    bnez TMP2, _CHECK_FOR_INTERRUPT_SAVE
    nop
    
    j _CHECK_FOR_INTERRUPT_EXIT
    addi TMP2, $zero, 0

_CHECK_FOR_INTERRUPT_SAVE:
    # set next interrupt time to be 1 frame ahead
    addi TMP3, CYCLES_RUN, 0 # TODO different timing with EI vs set interrupt flag
    sltu $at, CycleTo, TMP3
    # check to see if interrupt is new stopping point
    bnez $at, _CHECK_FOR_INTERRUPT_EXIT
    nop
    move CycleTo, TMP3
_CHECK_FOR_INTERRUPT_EXIT:
    # save next interrupt time
    sw TMP3, CPU_STATE_NEXT_INTERRUPT_TIME(CPUState)
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
    addi $sp, $sp, -4
    sw $ra, 0($sp)
    # check for video interrupts first 
    lw $at, CPU_STATE_NEXT_SCREEN(CPUState)
    sltu TMP2, CYCLES_RUN, $at
    bnez TMP2, _HANDLE_STOPPING_POINT_CHECK_TIMER
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
    sw $at, ST_CYCLE_TO($fp)
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
    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 4
    
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

    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 4
    
########################

_HANDLE_STOPPING_POINT_CHECK_INTERRUPT:
    lw $at, CPU_STATE_NEXT_INTERRUPT_TIME(CPUState)
    sltu $at, CYCLES_RUN, $at
    bnez $at, _HANDLE_STOPPING_POINT_BREAK

    li ADDR, 0x40 # load the base address for interrupt jumps
    # load interrupt requested
    lbu $at, CPU_STATE_NEXT_INTERRUPT(CPUState)
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

    la $at, ~0
    sw $at, CPU_STATE_NEXT_INTERRUPT_TIME(CPUState) # clear pending interrupt
    sb $zero, CPU_STATE_NEXT_INTERRUPT(CPUState)
    sb $zero, CPU_STATE_INTERRUPTS(CPUState) # disable interrupts
    
    jal CALCULATE_NEXT_STOPPING_POINT
    nop
    
    addi GB_SP, GB_SP, -2 # reserve space in stack
    andi GB_SP, GB_SP, 0xFFFF
    move VAL, GB_PC # set current PC to be saved
    jal SET_GB_PC
    move Param0, ADDR # set the new PC
    jal GB_DO_WRITE_16_CALL
    move ADDR, GB_SP # set the write address
    
    lw $ra, 0($sp)
    jr $ra
    addi $sp, $sp, 4
    
########################

_HANDLE_STOPPING_POINT_BREAK:
    j GB_BREAK_LOOP
    addi $sp, $sp, 4
    