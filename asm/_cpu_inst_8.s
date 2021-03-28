
### 0x8X
GB_ADD_A_B:
    j _ADD_TO_A
    addi Param0, GB_B, 0
GB_ADD_A_C:
    j _ADD_TO_A
    addi Param0, GB_C, 0
GB_ADD_A_D:
    j _ADD_TO_A
    addi Param0, GB_D, 0
GB_ADD_A_E:
    j _ADD_TO_A
    addi Param0, GB_E, 0
GB_ADD_A_H:
    j _ADD_TO_A
    addi Param0, GB_H, 0
GB_ADD_A_L:
    j _ADD_TO_A
    addi Param0, GB_L, 0
GB_ADD_A_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j _ADD_TO_A
    addi Param0, $v0, 0
GB_ADD_A_A:
    j _ADD_TO_A
    addi Param0, GB_A, 0
GB_ADC_A_B:
    j _ADC_TO_A
    addi Param0, GB_B, 0
GB_ADC_A_C:
    j _ADC_TO_A
    addi Param0, GB_C, 0
GB_ADC_A_D:
    j _ADC_TO_A
    addi Param0, GB_D, 0
GB_ADC_A_E:
    j _ADC_TO_A
    addi Param0, GB_E, 0
GB_ADC_A_H:
    j _ADC_TO_A
    addi Param0, GB_H, 0
GB_ADC_A_L:
    j _ADC_TO_A
    addi Param0, GB_L, 0
GB_ADC_A_HL:
    addi CYCLES_RUN, CYCLES_RUN, CYCLES_PER_INSTR # update cycles run
    sll ADDR, GB_H, 8 # load upper address
    jal GB_DO_READ # call read instruction
    or ADDR, ADDR, GB_L # load lower address
    j _ADC_TO_A
    addi Param0, $v0, 0
GB_ADC_A_A:
    j _ADC_TO_A
    addi Param0, GB_A, 0
    