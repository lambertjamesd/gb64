SECTION "entry",ROM0[$0000]

EntryPoint:
    JP SetRegisters

SECTION "text",ROM0[$0004]
    DB "DMG_BOOT"
SetRegisters:
    LD A, $87
    LDH [$ff14], A
    LD A, $77
    LDH [$ff24], A
    LD A, $80
    LDH [$ff26], A
    LD A, $91
    LDH [$ff40], A
    LD A, $00
    LDH [$ff4C], A
    JP Finish

SECTION "finish",ROM0[$00FC]
Finish:
    LD A, $1
    LDH [$ff50], A