#include "boot.h"

/*
 * ROM spec file
 */
beginseg
	name "code"
	flags BOOT OBJECT
	entry boot
	stack bootStack + STACKSIZE
	include "codesegment.o"
	include "$(ROOT)/usr/lib/PR/rspboot.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.fifo.o"
endseg

beginseg
	name "cfb"
	flags OBJECT
	address 0x80300000
	include "cfb.o"
endseg

beginseg
	name "static"
	flags OBJECT
	number STATIC_SEGMENT
	include "gfxinit.o"
endseg

beginseg
	name "rsp_cfb"
	flags OBJECT
	number CFB_SEGMENT
	include "rsp_cfb.o"
endseg

beginseg
	name "dmg_boot"
	flags RAW
	include "data/dmg_boot.bin"
endseg

beginseg
	name "cgb_bios"
	flags RAW
	include "data/cgb_bios.bin"
endseg

beginseg
	name "gbrom"
	flags RAW
	include "data/PokemonBlue.gb"
endseg

beginwave
	name "game"
	include "code"
	include "static"
	include "gbrom"
	include "dmg_boot"
	include "cgb_bios"
	include "cfb"
	include "rsp_cfb"
endwave
