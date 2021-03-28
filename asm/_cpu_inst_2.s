
### 0x2X
GB_JR_NZ:
    andi $at, GB_F, Z_FLAG # check z flag
    bnez $at, _SKIP_JR # if Z_FLAG != 0 skip jump
    nop
    j GB_JR
    nop
GB_LD_HL_D16:
    lbu GB_L, 0(PC_MEM_POINTER)
    lbu GB_H, 1(PC_MEM_POINTER)
    addi PC_MEM_POINTER, PC_MEM_POINTER, 2
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR * 2 # update cycles run
    j DECODE_NEXT
    addi GB_PC, GB_PC, 2
GB_LDI_HL_A:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    add VAL, GB_A, 0 # write the value to store
    sll ADDR, GB_H, 8 # write upper address
    or ADDR, ADDR, GB_L # write lower address

    addi GB_L, ADDR, 1 # increment address
    srl GB_H, GB_L, 8 # store upper bits
    andi GB_L, GB_L, 0xFF # mask lower bits
    andi GB_H, GB_H, 0xFF # mask upper bits bits
    j GB_DO_WRITE # call store subroutine
    nop
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
GB_DEC_H:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_H, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_H, Param0, 0 # move register back from call parameter
GB_LD_H_D8:
    lbu GB_H, 0(PC_MEM_POINTER)
    addi PC_MEM_POINTER, PC_MEM_POINTER, 1
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_PC, GB_PC, 1
GB_DAA:
    j _GB_DAA
    nop
GB_JR_Z:
    andi $at, GB_F, Z_FLAG # check z flag
    beqz $at, _SKIP_JR # skip jump if not zero
    nop
    j GB_JR
    nop
GB_ADD_HL_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll Param0, GB_H, 8 # load high order bits
    j _ADD_TO_HL
    or Param0, Param0, GB_L # load low order bits
GB_LDI_A_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    or ADDR, ADDR, GB_L # load lower address
    addi GB_L, ADDR, 1 # increment L
    jal GB_DO_READ # call read instruction
    srl GB_H, GB_L, 8 # store incremented H
    addi GB_A, $v0, 0 # store result into a
    andi GB_L, GB_L, 0xFF # mask lower bits
    j DECODE_NEXT
    andi GB_H, GB_H, 0xFF # mask upper bits bits
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
GB_DEC_L:
    jal GB_DEC # call decrement high bit
    addi Param0, GB_L, 0 # move register to call parameter
    j DECODE_NEXT
    addi GB_L, Param0, 0 # move register back from call parameter
GB_LD_L_D8:
    lbu GB_L, 0(PC_MEM_POINTER)
    addi PC_MEM_POINTER, PC_MEM_POINTER, 1
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    j DECODE_NEXT
    addi GB_PC, GB_PC, 1
GB_CPL:
    xori GB_A, GB_A, 0xFF
    j DECODE_NEXT
    set_flags N_FLAG | H_FLAG
    