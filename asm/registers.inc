
.eqv REG_TAC, 0xFF07

.eqv REG_TAC_STOP_BIT, 0x40
.eqv REG_TAC_CLOCK_SELECT, 0x3

.eqv REG_TIMA, 0xFF05
.eqv REG_TMA, 0xFF06

.eqv INTERRUPTS_REQUESTED, 0xFF0F

.eqv INTERRUPT_V_BLANK, 0x01
.eqv INTERRUPT_LCD_STAT, 0x02
.eqv INTERRUPT_TIMER, 0x04
.eqv INTERRUPT_SERIAL, 0x08
.eqv INTERRUPT_JOYPAD, 0x10

.macro read_register_direct target, register
    lbu \target, (\register+MEMORY_MISC_START-MM_REGISTER_START)(Memory)
.endm

.macro write_register_direct target, register
    sb \target, (\register+MEMORY_MISC_START-MM_REGISTER_START)(Memory)
.endm