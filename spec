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
	include "$(ROOT)/usr/lib/PR/gspL3DEX2.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspL3DEX2.xbus.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.xbus.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.NoN.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.NoN.xbus.o"
	include "memory.o"
endseg

beginseg
	name "zbuffer"
	flags OBJECT
	address 0x80118000
	include "zbuffer.o"
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
	include "gfxstatic.o"
endseg

beginseg
	name "rsp_cfb"
	flags OBJECT
	number CFB_SEGMENT
	include "rsp_cfb.o"
endseg

beginwave
	name "game"
	include "code"
	include "static"
	include "cfb"
	include "rsp_cfb"
	include "zbuffer"
endwave
