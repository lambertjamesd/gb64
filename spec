#include "boot.h"

#define USE_PLACEHOLDER 0

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
	name "ppu_text"
	flags RAW
	include "bin/rsp/ppu.text"
endseg

beginseg
	name "ppu_data"
	flags RAW
	include "bin/rsp/ppu.text.dat"
endseg

#if USE_PLACEHOLDER
beginseg
	name "dmg_boot"
	flags RAW
	include "data/dmg_boot_placeholder.bin"
endseg

beginseg
	name "cgb_bios"
	flags RAW
	include "data/cgb_bios_placeholder.bin"
endseg

beginseg
	name "gbrom"
	flags RAW
	include "data/rom_placeholder.gb"
endseg

#else  // USE_PLACEHOLDER

beginseg
	name "dmg_boot"
	flags RAW
	//include "data/dmg_boot.bin"
	include "data/dmg_boot_placeholder.bin"
endseg

beginseg
	name "cgb_bios"
	flags RAW
	include "data/cgb_bios.bin"
endseg

beginseg
	name "gbrom"
	flags RAW
	include "data/AdventureIslandII.gb"
endseg

#endif // USE_PLACEHOLDER

beginwave
	name "game"
	include "code"
	include "static"
	include "ppu_text"
	include "ppu_data"
	include "dmg_boot"
	include "cgb_bios"
	include "rsp_cfb"
	include "gbrom"
endwave