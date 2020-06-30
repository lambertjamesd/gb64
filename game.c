/**************************************************************************
 *                                                                        *
 *               Copyright (C) 1995, Silicon Graphics, Inc.               *
 *                                                                        *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright  law.  They  may not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *                                                                        *
 *************************************************************************/

/*---------------------------------------------------------------------*
        Copyright (C) 1997,1998 Nintendo. (Originated by SGI)
        
        $RCSfile: game.c,v $
        $Revision: 1.9 $
        $Date: 1999/04/16 07:11:47 $
 *---------------------------------------------------------------------*/

/*
 * File:  game.c
 *
 *
 */

#include <ultra64.h>
#include <PR/ramrom.h>		/* needed for argument passing into the app */
#include <assert.h>

#include "memory.h"
#include "game.h"
#include "static.h"
#include "controller.h"
#include "font_ext.h"
#include "src/gameboy.h"
#include "src/graphics.h"
#include "src/test/test.h"
#include "src/debug_out.h"
#include "render.h"
#include "src/debugger.h"
#include "src/mainmenu.h"

#define RUN_TESTS 0

/*
 * This is the main routine of the app.
 */
void
game(void)
{
    OSContPad	**pad;
    char 	*cstring;
    char	str[200];
	int loop;
	int lastButton;
	int accumulatedTime = 0;
	u32 lastTime;
	u32 lastDrawTime;
	void* debugWrite;

	loop = 0;
	lastButton = 0;

	lastDrawTime = 0;
	lastTime = osGetCount();
	gGameboy.cpu.cyclesRun = 0;
#if RUN_TESTS
	runTests(str);
#endif

	debugInfo(str);

	initGameboy(&gGameboy, &gGBRom);

	initDebugMenu(&gDebugMenu, &gGameboy.cpu, &gGameboy.memory);

	gGameboy.memory.vram.colorPalletes[0] = 0b1100011110110100;
	gGameboy.memory.vram.colorPalletes[1] = 0b1000111000011100;
	gGameboy.memory.vram.colorPalletes[2] = 0b0011001101010100;
	gGameboy.memory.vram.colorPalletes[3] = 0b0000100011001000;
	
	gGameboy.memory.vram.colorPalletes[OBJ_PALLETE_INDEX_START + 0] = 0b1100011110110100;
	gGameboy.memory.vram.colorPalletes[OBJ_PALLETE_INDEX_START + 1] = 0b1000111000011100;
	gGameboy.memory.vram.colorPalletes[OBJ_PALLETE_INDEX_START + 2] = 0b0011001101010100;
	gGameboy.memory.vram.colorPalletes[OBJ_PALLETE_INDEX_START + 3] = 0b0000100011001000;
	gGameboy.memory.vram.colorPalletes[OBJ_PALLETE_INDEX_START + 4] = 0b1100011110110100;
	gGameboy.memory.vram.colorPalletes[OBJ_PALLETE_INDEX_START + 5] = 0b1000111000011100;
	gGameboy.memory.vram.colorPalletes[OBJ_PALLETE_INDEX_START + 6] = 0b0011001101010100;
	gGameboy.memory.vram.colorPalletes[OBJ_PALLETE_INDEX_START + 7] = 0b0000100011001000;

	zeroMemory(cfb, sizeof(u16) * 2 * SCREEN_WD * SCREEN_HT);

	debugWrite = (void*)(0x80700000 - 12);
	*((u32*)debugWrite) = (int)&gGameboy.memory;
	debugWrite = (void*)(0x80700000 - 16);
	*((u32*)debugWrite) = (int)gGameboy.memory.rom;
	debugWrite = (void*)(0x80700000 - 8);
	*((u32*)debugWrite) = (int)&gGameboy.cpu;

	OSTime startTime = osGetCount();

	initMainMenu(&gMainMenu);

	int frames = 0;

	// clearDebugOutput();
	DEBUG_PRINT_F("\n%X\n%X\n", 0, 0);

    /*
     * Main game loop
     */
    /*osSyncPrintf("Use the L button and crosshair for menu options.\n");*/
    while (1) {
		pad = ReadController(0);

        if ((pad[0]->button & L_CBUTTONS) && (~lastButton & L_CBUTTONS) && (pad[0]->button & R_TRIG) && (pad[0]->button & L_TRIG))
        {
            useDebugger(&gGameboy.cpu, &gGameboy.memory);
        }

		lastButton = pad[0]->button;

		u32 currentTime = osGetCount();
		u32 frameTime = currentTime - lastTime;
		lastTime = currentTime;


		accumulatedTime += frameTime;

		// time s  1024*1024 cycles/s 60 frames/s

		cstring=str;

		
#if !RUN_TESTS
		lastDrawTime = -osGetCount();

		// gameboy is 60 fps N64 is 30
		// we need to emulator two frames
		// for every one frame rendered
		
		// clearDebugOutput();

		
		loop = MAX_FRAME_SKIP;
		while (accumulatedTime > CPU_TICKS_PER_FRAME && loop > 0)
		{
			if (!isMainMenuOpen(&gMainMenu))
			{
				handleGameboyInput(&gGameboy, pad[0]);
			}
			emulateFrame(&gGameboy, NULL);
			pad = ReadController(0);
			accumulatedTime -= CPU_TICKS_PER_FRAME;
			--loop;
			++frames;
		}

		while (accumulatedTime > CPU_TICKS_PER_FRAME)
		{
			accumulatedTime -= CPU_TICKS_PER_FRAME;
		}

		if (!isMainMenuOpen(&gMainMenu))
		{
			handleGameboyInput(&gGameboy, pad[0]);
		}
		emulateFrame(&gGameboy, getColorBuffer());
		accumulatedTime -= CPU_TICKS_PER_FRAME;
		finishAudioFrame(&gGameboy.memory);
		++frames;

		lastDrawTime += osGetCount();
		// sprintf(str, "Cycles run %d\nFrame Time %d\nEmu time %d\n%X", 
		// 	gGameboy.cpu.cyclesRun, 
		// 	(int)OS_CYCLES_TO_USEC(frameTime) / 1000, 
		// 	(int)OS_CYCLES_TO_USEC(lastDrawTime) / 1000,
		// 	renderSprites
		// );
		// debugInfo(str);
#endif
		preRenderFrame();
		
    	gSPDisplayList(glistp++, gDrawScreen);

		renderDebugLog();

		updateMainMenu(&gMainMenu, pad[0]);
		renderMainMenu(&gMainMenu);

		finishRenderFrame();
    }
}
