SECTION "entry",ROM0[$0000]

EntryPoint:
    JP SetRegisters

SECTION "text",ROM0[$0004]
    DB "CGB_BIOS"
SetRegisters:
    LD A, $87
    LDH [$ff14], A
    LD A, $77
    LDH [$ff24], A
    LD A, $80
    LDH [$ff26], A
    LD A, $91
    LDH [$ff40], A
    LD A, $C0
    LDH [$ff4C], A
    LD A, $00
    LDH [$ff70], A
    JP Finish

SECTION "finish",ROM0[$00FC]
Finish:
    LD A, $11
    LDH [$ff50], A

SECTION "padding",ROM0[$08FF]
    DB 0