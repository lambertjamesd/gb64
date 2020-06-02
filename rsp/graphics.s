
MAIN:
    beq $0, $0, MAIN
    nop
LABEL:
    nop
    nop
    j LABEL
    nop
    
    .data
    .byte 0x48
    .byte 0x65
    .byte 0x6c
    .byte 0x6c
    .byte 0x6f
    .byte 0x20
    .byte 0x57
    .byte 0x6f
    .byte 0x72
    .byte 0x64
    .byte 0x21