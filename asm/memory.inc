
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
.set CYCLES_RUN, $t2
.set PC_MEMORY_BANK, $t3

.set ADDR, $t4
.set VAL,  $t5
.set TMP2,  $t6
.set TMP3,  $t7
.set TMP4,  $t8

.set CPUState, $a0
.set Memory, $a1
.set CycleTo, $a2

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
.eqv CPU_STATE_NEXT_INTERRUPT, 0xE
.eqv CPU_STATE_RUN_UNTIL_FRAME, 0xF
.eqv CPU_STATE_CYCLES_RUN, 0x10
.eqv CPU_STATE_NEXT_TIMER, 0x14
.eqv CPU_STATE_NEXT_SCREEN, 0x18
.eqv CPU_STATE_NEXT_INTERRUPT_TIME, 0x1C
.eqv CPU_STATE_UNSCALED_CYCLES_RUN, 0x20

.eqv MEMORY_ADDR_TABLE, 0x00
.eqv MEMORY_REGISTER_TABLE, 0x40
.eqv MEMORY_BANK_SWITCHING, 0x60
.eqv MEMORY_MISC_START, 0x64
.eqv MEMORY_RAM_START, 0x264
.eqv MEMORY_CART_RAM, 0x8264
.eqv MEMORY_VRAM, 0x8268
.eqv MEMORY_BG_PAL, 0xC268
.eqv MEMORY_OBJ_PAL, 0xC2A8
.eqv MEMORY_ROM, 0xC2E8

.eqv MEMORY_VRAM_BANK_INDEX, 0x8
.eqv MEMORY_RAM_BANK_INDEX, 0xD

.eqv MM_REGISTER_START, 0xFE00

.eqv Z_FLAG, 0x80
.eqv N_FLAG, 0x40
.eqv H_FLAG, 0x20
.eqv C_FLAG, 0x10

.eqv STACK_SIZE, 0x34
.eqv ST_RA, 0x0
# hole here
.eqv ST_S0, 0x8
.eqv ST_S1, 0xC
.eqv ST_S2, 0x10
.eqv ST_S3, 0x14
.eqv ST_S4, 0x18
.eqv ST_S5, 0x1C
.eqv ST_S6, 0x20
.eqv ST_S7, 0x24
.eqv ST_FP, 0x28
.eqv ST_CYCLE_TO, 0x2C
.eqv ST_NEXT_V_INT, 0x30

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

.eqv _C_CALLBACK_FRAME_SIZE, 0x14

.macro save_state_on_stack
    addi $sp, $sp, -_C_CALLBACK_FRAME_SIZE
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

    sw CYCLES_RUN, CPU_STATE_CYCLES_RUN(CPUState)

    sw CPUState, 0x00($sp)
    sw Memory, 0x04($sp)
    sw CycleTo, 0x08($sp)
    sw $ra, 0x0C($sp)
    sw $fp, 0x10($sp)
.endm

.macro restore_state_from_stack
    lw CPUState, 0x00($sp)
    lw Memory, 0x04($sp)
    lw CycleTo, 0x08($sp)
    lw $fp, 0x10($sp)

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
    
    lw CYCLES_RUN, CPU_STATE_CYCLES_RUN(CPUState)

    # in case the program counter is inside the bank of 
    # memory that was switched
    jal SET_GB_PC
    # make sure GB_PC doesn't match Param0 to force bank load
    xori GB_PC, Param0, 0xFFFF 

    lw $ra, 0x0C($sp)

    addi $sp, $sp, _C_CALLBACK_FRAME_SIZE
.endm

.macro call_c_fn name
    lui $at, %hi(\name)
    addiu $at, $at, %lo(\name)
    jalr $ra, $at
.endm
