#!smake -k
# --------------------------------------------------------------------
#        Copyright (C) 1997,1998 Nintendo. (Originated by SGI)
#
#        $RCSfile: Makefile,v $
#        $Revision: 1.9 $
#        $Date: 1999/04/02 10:10:22 $
# --------------------------------------------------------------------
#
# Makefile for game template
#
#

include $(ROOT)/usr/include/make/PRdefs

WARNING_FLAGS = -Werror=implicit-function-declaration

RSP2DWARF = /home/james/go/src/github.com/lambertjamesd/rsp2dwarf/rsp2dwarf

# INCLUDE_DEBUGGER = TRUE

ifeq ($(INCLUDE_DEBUGGER), TRUE)
DEBUG_FLAGS = -g -DUSE_DEBUGGER
DEBUG_FILES = debugger/debugger.c debugger/serial.c debugger/rsp.c
else
DEBUG_FILES = 
DEBUG_FLAGS =
endif

FINAL = YES
ifeq ($(FINAL), YES)
OPTIMIZER       = $(DEBUG_FLAGS) -g -O2 -std=gnu99 -Werror $(WARNING_FLAGS)
LCDEFS          = -D_FINALROM -DNDEBUG -DF3DEX_GBI_2
ASFLAGS         = -mabi=32
N64LIB          = -lultra_rom
# N64_ASFLAGS     = --gen-debug -call_nonpic -march=r4300 -mtune=vr4300 -mabi=32 -mno-shared --defsym DEBUG=0
else
OPTIMIZER       = $(DEBUG_FLAGS) -g -std=gnu99 -Werror $(WARNING_FLAGS)
LCDEFS          = -DDEBUG -DF3DEX_GBI_2
ASFLAGS         = -mabi=32
N64LIB          = -lultra_rom
# N64_ASFLAGS     = --gen-debug -call_nonpic -march=r4300 -mtune=vr4300 -mabi=32 -mno-shared --defsym DEBUG=1
endif

TARGET_BASE_NAME =	bin/gb

LD_SCRIPT	= gb64.ld
CP_LD_SCRIPT	= build/gb64.ld

BOOT		=	/usr/lib/n64/PR/bootcode/boot.6102
BOOT_OBJ	=	build/boot.6102.o

COMPSOURCE = gzip/adler32.c \
       gzip/crc32.c \
       gzip/defl_static.c \
       gzip/genlz77.c \
       gzip/tinfgzip.c \
       gzip/tinflate.c \
       gzip/tinfzlib.c

HFILES =	boot.h game.h controller.h font.h font_ext.h \
		gbfont_img.h static.h \
		src/test/cpu_test.h    \
		src/cpu.h              \
		src/memory_map.h

CODEFILES   =	boot.c game.c controller.c dram_stack.c \
       src/assert.c                          \
       src/test/cpu_test.c                   \
       src/test/cpu_tests_0.c                \
       src/test/cpu_tests_1.c                \
       src/test/cpu_tests_2.c                \
       src/test/cpu_tests_3.c                \
       src/test/cpu_tests_4_7.c              \
       src/test/cpu_tests_8_9.c              \
       src/test/cpu_tests_A_B.c              \
       src/test/cpu_tests_C.c                \
       src/test/cpu_tests_D.c                \
       src/test/cpu_tests_E.c                \
       src/test/cpu_tests_F.c                \
       src/test/cpu_tests_prefix_cb.c        \
       src/test/register_test.c              \
       src/test/interrupt_test.c             \
       src/cpu.c                             \
       src/rom.c                             \
       src/memory_map.c                      \
       src/gameboy.c                         \
       src/graphics.c                        \
       memory.c                              \
       memory_test.c                         \
       src/test/test.c                       \
       src/debug_out.c                       \
       src/audio.c                           \
       src/debugger.c                        \
       render.c                              \
       src/menu.c                            \
       src/decoder.c                         \
       src/mainmenu.c                        \
       src/rspppu.c                          \
       src/inputmapping.c                    \
       src/graphicsmenu.c                    \
       src/clockmenu.c                       \
       src/erasemenu.c                       \
       src/upgrade.c                         \
       src/save.c                            \
       src/sprite.c                          \
       src/spritefont.c                      \
       src/faulthandler.c                    \
	   src/polyfill.c                        \
       tex/textures.c                        \
       $(DEBUG_FILES)                        \
       $(COMPSOURCE)

S_FILES = asm/cpu.s asm/entry.s asm/rom_header.s asm/data.s asm/data_placeholder.s

CODEOBJECTS = $(patsubst %.c, build/%.o, $(CODEFILES)) build/asm/cpu.o

ASMOBJECTS = $(patsubst %.s, build/%.o, $(S_FILES))

DATAFILES   =	gfxinit.c

DATAOBJECTS =	$(patsubst %.c, build/%.o, $(DATAFILES))

CODESEGMENT =	build/codesegment.o

OBJECTS =	$(CODESEGMENT) $(DATAOBJECTS) $(ASMOBJECTS) $(BOOT_OBJ) data/cgb_bios_placeholder.bin data/dmg_boot_placeholder.bin bin/rsp/ppu.o

build/asm/data.o build/asm/data_placeholder.o: data/cgb_bios_placeholder.bin data/dmg_boot_placeholder.bin

LCDEFS +=	$(HW_FLAGS)
LCINCS =	-I. -I/usr/include/n64/PR -I/usr/include/n64 -I/usr/include/n64/nustd
LCOPTS =	-G 0
LDFLAGS =	-L/usr/lib/n64 $(N64LIB)  -L$(N64_LIBGCCDIR) -lgcc

data/cgb_bios_placeholder.bin: data/cgb_bios_placeholder.asm
	rgbasm data/cgb_bios_placeholder.asm -o data/cgb_bios_placeholder.o
	rgblink --nopad -o data/cgb_bios_placeholder.bin data/cgb_bios_placeholder.o
	
data/dmg_boot_placeholder.bin: data/dmg_boot_placeholder.asm
	rgbasm data/dmg_boot_placeholder.asm -o data/dmg_boot_placeholder.o
	rgblink --nopad -o data/dmg_boot_placeholder.bin data/dmg_boot_placeholder.o

RSPFILES = $(wildcard ./rsp/*.s)

bin/rsp/ppu bin/rsp/ppu.dat: $(RSPFILES)
	mkdir -p bin/rsp
	rspasm -o bin/rsp/ppu rsp/ppu.s

bin/rsp/ppu.o: bin/rsp/ppu bin/rsp/ppu.dat
	$(RSP2DWARF) bin/rsp/ppu -o bin/rsp/ppu.o -n ppu
	$(RSP2DWARF) bin/rsp/ppu -o bin/rsp/ppu.debug.o -n ppu -g
	
bin/rsp/ppu.tvd: bin/rsp/ppu bin/rsp/ppu.dat
	rsp2elf bin/rsp/ppu

default:	$(TARGET_BASE_NAME).z64

build/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MM $^ -MF "$(@:.o=.d)" -MT"$@"
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.o: %.s
	@mkdir -p $(@D)
	$(AS) -Wa,-Iasm -o $@ $<

asm/cpu.o: asm/memory.inc asm/registers.inc asm/_branch.s \
       asm/_cpu_inst_prefix.s asm/_math.s asm/_stopping_point.s \
       asm/_memory.s \
       asm/_registers.s \
       asm/_debug.s \
       asm/_mbc.s \
       asm/_mbc7.s \
       asm/_cpu_inst_0.s asm/_cpu_inst_1.s asm/_cpu_inst_2.s asm/_cpu_inst_3.s \
       asm/_cpu_inst_4.s asm/_cpu_inst_5.s asm/_cpu_inst_6.s asm/_cpu_inst_7.s \
       asm/_cpu_inst_8.s asm/_cpu_inst_9.s asm/_cpu_inst_A.s asm/_cpu_inst_B.s \
       asm/_cpu_inst_C.s asm/_cpu_inst_D.s asm/_cpu_inst_E.s asm/_cpu_inst_F.s

include $(COMMONRULES)

$(BOOT_OBJ): $(BOOT)
	$(OBJCOPY) -I binary -B mips -O elf32-bigmips $< $@

$(CP_LD_SCRIPT): $(LD_SCRIPT)
	cpp -P -Wno-trigraphs $(LCDEFS) -DCODE_SEGMENT=$(CODESEGMENT) -o $@ $<

$(CODESEGMENT):	$(CODEOBJECTS)
		$(LD) -o $(CODESEGMENT) -r $(CODEOBJECTS) $(LDFLAGS)

$(TARGET_BASE_NAME).z64: $(CODESEGMENT) $(OBJECTS) $(CP_LD_SCRIPT)
	$(LD) -L. -T $(CP_LD_SCRIPT) -Map $(TARGET_BASE_NAME).map -o $(TARGET_BASE_NAME).elf
	$(OBJCOPY) --pad-to=0x100000 --gap-fill=0xFF $(TARGET_BASE_NAME).elf $(TARGET_BASE_NAME).z64 -O binary
	makemask $(TARGET_BASE_NAME).z64

cleanall: clean
	rm -f $(CODEOBJECTS) $(OBJECTS)
	rm -rf bin/rsp
	rm -rfd build/

rsp/%.o: rsp/%.s
	$(RSPASM) $< -o $@

romwrapper/gb.n64.js: $(TARGET_BASE_NAME).z64
	echo "const gGB64Base64 = \`" > romwrapper/gb.n64.js
	base64 $(TARGET_BASE_NAME).z64 >> romwrapper/gb.n64.js
	echo "\`.trim()" >> romwrapper/gb.n64.js

bin/objdump.txt: $(TARGET_BASE_NAME).z64
	mips64-elf-objdump -S game.out > bin/objdump.txt
