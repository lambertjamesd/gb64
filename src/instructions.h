
#ifndef _INSTRUCTIONS_H
#define _INSTRUCTIONS_H

#define CPU_NOP         0x00
#define CPU_LD_BC_d16   0x01
#define CPU_LD_BC_A     0x02
#define CPU_INC_BC      0x03
#define CPU_INC_B       0x04
#define CPU_DEC_B       0x05
#define CPU_LD_B_d8     0x06
#define CPU_RLCA        0x07
#define CPU_LD_a16_SP   0x08
#define CPU_ADD_HL_BC   0x09
#define CPU_LD_A_BC     0x0A
#define CPU_DEC_BC      0x0B
#define CPU_INC_C       0x0C
#define CPU_DEC_C       0x0D
#define CPU_LD_C_d8     0x0E
#define CPU_RRCA        0x0F

#define CPU_STOP        0x10
#define CPU_LD_DE_d16   0x11
#define CPU_LD_DE_A     0x12
#define CPU_INC_DE      0x13
#define CPU_INC_D       0x14
#define CPU_DEC_D       0x15
#define CPU_LD_D_d8     0x16
#define CPU_RLA         0x17
#define CPU_JR          0x18
#define CPU_ADD_HL_DE   0x19
#define CPU_LD_A_DE     0x1A
#define CPU_DEC_DE      0x1B
#define CPU_INC_E       0x1C
#define CPU_DEC_E       0x1D
#define CPU_LD_E_d8     0x1E
#define CPU_RRA         0x1F

#define CPU_JR_NZ       0x20
#define CPU_LD_HL_d16   0x21
#define CPU_LDI_HL_A    0x22
#define CPU_INC_HL      0x23
#define CPU_INC_H       0x24
#define CPU_DEC_H       0x25
#define CPU_LD_H_d8     0x26
#define CPU_DAA         0x27
#define CPU_JR_Z        0x28
#define CPU_ADD_HL_HL   0x29
#define CPU_LDI_A_HL    0x2A
#define CPU_DEC_HL      0x2B
#define CPU_INC_L       0x2C
#define CPU_DEC_L       0x2D
#define CPU_LD_L_d8     0x2E
#define CPU_CPL         0x2F

#define CPU_JR_NC       0x30
#define CPU_LD_SP_d16   0x31
#define CPU_LDD_HL_A    0x32
#define CPU_INC_SP      0x33
#define CPU_INC_HL_ADDR 0x34
#define CPU_DEC_HL_ADDR 0x35
#define CPU_LD_HL_d8    0x36
#define CPU_SCF         0x37
#define CPU_JR_C        0x38
#define CPU_ADD_HL_SP   0x39
#define CPU_LDD_A_HL    0x3A
#define CPU_DEC_SP      0x3B
#define CPU_INC_A       0x3C
#define CPU_DEC_A       0x3D
#define CPU_LD_A_d8     0x3E
#define CPU_CCF         0x3F

#define CPU_LD_B_B      0x40
#define CPU_LD_B_C      0x41
#define CPU_LD_B_D      0x42
#define CPU_LD_B_E      0x43
#define CPU_LD_B_H      0x44
#define CPU_LD_B_L      0x45
#define CPU_LD_B_HL     0x46
#define CPU_LD_B_A      0x47
#define CPU_LD_C_B      0x48
#define CPU_LD_C_C      0x49
#define CPU_LD_C_D      0x4A
#define CPU_LD_C_E      0x4B
#define CPU_LD_C_H      0x4C
#define CPU_LD_C_L      0x4D
#define CPU_LD_C_HL     0x4E
#define CPU_LD_C_A      0x4F

#define CPU_LD_D_B      0x50
#define CPU_LD_D_C      0x51
#define CPU_LD_D_D      0x52
#define CPU_LD_D_E      0x53
#define CPU_LD_D_H      0x54
#define CPU_LD_D_L      0x55
#define CPU_LD_D_HL     0x56
#define CPU_LD_D_A      0x57
#define CPU_LD_E_B      0x58
#define CPU_LD_E_C      0x59
#define CPU_LD_E_D      0x5A
#define CPU_LD_E_E      0x5B
#define CPU_LD_E_H      0x5C
#define CPU_LD_E_L      0x5D
#define CPU_LD_E_HL     0x5E
#define CPU_LD_E_A      0x5F

#define CPU_LD_H_B      0x60
#define CPU_LD_H_C      0x61
#define CPU_LD_H_D      0x62
#define CPU_LD_H_E      0x63
#define CPU_LD_H_H      0x64
#define CPU_LD_H_L      0x65
#define CPU_LD_H_HL     0x66
#define CPU_LD_H_A      0x67
#define CPU_LD_L_B      0x68
#define CPU_LD_L_C      0x69
#define CPU_LD_L_D      0x6A
#define CPU_LD_L_E      0x6B
#define CPU_LD_L_H      0x6C
#define CPU_LD_L_L      0x6D
#define CPU_LD_L_HL     0x6E
#define CPU_LD_L_A      0x6F

#define CPU_LD_HL_B     0x70
#define CPU_LD_HL_C     0x71
#define CPU_LD_HL_D     0x72
#define CPU_LD_HL_E     0x73
#define CPU_LD_HL_H     0x74
#define CPU_LD_HL_L     0x75
#define CPU_HALT        0x76
#define CPU_LD_HL_A     0x77
#define CPU_LD_A_B      0x78
#define CPU_LD_A_C      0x79
#define CPU_LD_A_D      0x7A
#define CPU_LD_A_E      0x7B
#define CPU_LD_A_H      0x7C
#define CPU_LD_A_L      0x7D
#define CPU_LD_A_HL     0x7E
#define CPU_LD_A_A      0x7F

#define CPU_ADD_A_B     0x80
#define CPU_ADC_A_B     0x88

#define CPU_SUB_A_B     0x90
#define CPU_SBC_A_B     0x98

#define CPU_AND_A_B     0xA0
#define CPU_XOR_A_B     0xA8

#define CPU_OR_A_B      0xB0
#define CPU_CP_A_B      0xB8

#define CPU_RET_NZ      0xC0
#define CPU_POP_BC      0xC1
#define CPU_JP_NZ_a16   0xC2
#define CPU_JP_a16      0xC3
#define CPU_CALL_NZ     0xC4
#define CPU_PUSH_BC     0xC5
#define CPU_ADD_A_d8    0xC6
#define CPU_RST_00H     0xC7
#define CPU_RET_Z       0xC8
#define CPU_RET         0xC9
#define CPU_JP_Z_a16    0xCA
#define CPU_PREFIX_CB   0xCB
#define CPU_CALL_Z      0xCC
#define CPU_CALL        0xCD
#define CPU_ADC_A_d8    0xCE
#define CPU_RST_08H     0xCF

#define CPU_RET_NC      0xD0
#define CPU_POP_DE      0xD1
#define CPU_JP_NC_a16   0xD2
#define CPU_CALL_NC     0xD4
#define CPU_PUSH_DE     0xD5
#define CPU_SUB_A_d8    0xD6
#define CPU_RST_10H     0xD7
#define CPU_RET_C       0xD8
#define CPU_RETI        0xD9
#define CPU_JP_C_a16    0xDA
#define CPU_CALL_C      0xDC
#define CPU_SBC_A_d8    0xDE
#define CPU_RST_18H     0xDF

#define CPU_LDH_a8_A    0xE0
#define CPU_POP_HL      0xE1
#define CPU_LD_C_ADDR_A 0xE2
#define CPU_PUSH_HL     0xE5
#define CPU_AND_A_d8    0xE6
#define CPU_RST_20H     0xE7
#define CPU_ADD_SP_r8   0xE8
#define CPU_JP_HL       0xE9
#define CPU_LD_a16_A    0xEA
#define CPU_XOR_A_d8    0xEE
#define CPU_RST_28H     0xEF

#define CPU_LDH_A_a8    0xF0
#define CPU_POP_AF      0xF1
#define CPU_LD_A_C_ADDR 0xF2
#define CPU_DI          0xF3
#define CPU_PUSH_AF     0xF5
#define CPU_OR_A_d8     0xF6
#define CPU_RST_30H     0xF7
#define CPU_LD_HL_SP_r8 0xF8
#define CPU_SP_HL       0xF9
#define CPU_LD_A_a16    0xFA
#define CPU_EI          0xFB
#define CPU_CP_A_d8     0xFE
#define CPU_RST_38H     0xFF

#define CPU_CB_RLC      0x00
#define CPU_CB_RRC      0x08
#define CPU_CB_RL       0x10
#define CPU_CB_RR       0x18
#define CPU_CB_SLA      0x20
#define CPU_CB_SRA      0x28
#define CPU_CB_SWAP     0x30
#define CPU_CB_SRL      0x38
#define CPU_CB_BIT      0x40
#define CPU_CB_RES      0x80
#define CPU_CB_SET      0xC0

#endif