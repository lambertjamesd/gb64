.include "macros.inc"

.section .data

glabel _dmg_bootSegmentRomStart
.incbin "data/dmg_boot_placeholder.bin"
.balign 16
glabel _dmg_bootSegmentRomEnd

glabel _cgb_biosSegmentRomStart
.incbin "data/cgb_bios_placeholder.bin"
.balign 16
glabel _cgb_biosSegmentRomEnd

glabel _gbromSegmentRomStart
.incbin "data/PokemonYellow.gb"
.balign 16
glabel _gbromSegmentRomEnd