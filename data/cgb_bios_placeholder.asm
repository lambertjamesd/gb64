SECTION "entry",ROM0[$0000]

EntryPoint:
    JP $00FC

SECTION "text",ROM0[$0004]
    DB "CGB_BIOS"

SECTION "finish",ROM0[$00FC]
    LD A, $11
    LDH [$ff50], A

SECTION "padding",ROM0[$08FF]
    DB 0