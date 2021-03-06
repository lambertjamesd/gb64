SECTION "entry",ROM0[$0000]

EntryPoint:
    JP $00FC

SECTION "text",ROM0[$0004]
    DB "DMG_BOOT"

SECTION "finish",ROM0[$00FC]
    LD A, $1
    LDH [$ff50], A