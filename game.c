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
	int loop, offset, color;
	int lastButton;
	int accumulatedTime;
	OSTime lastTime;
	OSTime lastDrawTime;
	void* debugWrite;

	loop = 0;
	offset = 0;
	color = 0;
	lastButton = 0;

	lastDrawTime = 0;
	lastTime = osGetTime();
	gGameboy.cpu.cyclesRun = 0;
	clearDebugOutput();
#if RUN_TESTS
	runTests(str);
#endif

	debugInfo(str);

	initGameboy(&gGameboy, &gGBRom);

	initDebugMenu(&gDebugMenu, &gGameboy.cpu, &gGameboy.memory);

	gGameboy.memory.vram.bgColorPalletes[0] = 0b1100011110110100;
	gGameboy.memory.vram.bgColorPalletes[1] = 0b1000111000011100;
	gGameboy.memory.vram.bgColorPalletes[2] = 0b0011001101010100;
	gGameboy.memory.vram.bgColorPalletes[3] = 0b0000100011001000;
	
	gGameboy.memory.vram.objColorPalletes[0] = 0b1100011110110100;
	gGameboy.memory.vram.objColorPalletes[1] = 0b1000111000011100;
	gGameboy.memory.vram.objColorPalletes[2] = 0b0011001101010100;
	gGameboy.memory.vram.objColorPalletes[3] = 0b0000100011001000;
	gGameboy.memory.vram.objColorPalletes[4] = 0b1100011110110100;
	gGameboy.memory.vram.objColorPalletes[5] = 0b1000111000011100;
	gGameboy.memory.vram.objColorPalletes[6] = 0b0011001101010100;
	gGameboy.memory.vram.objColorPalletes[7] = 0b0000100011001000;

	zeroMemory(cfb, sizeof(u16) * 2 * SCREEN_WD * SCREEN_HT);

	debugWrite = (void*)(0x80700000 - 12);
	*((u32*)debugWrite) = (int)&gGameboy.memory;
	debugWrite = (void*)(0x80700000 - 16);
	*((u32*)debugWrite) = (int)gGameboy.memory.rom;
	debugWrite = (void*)(0x80700000 - 8);
	*((u32*)debugWrite) = (int)&gGameboy.cpu;

	OSTime startTime = osGetTime();

    /*
     * Main game loop
     */
    /*osSyncPrintf("Use the L button and crosshair for menu options.\n");*/
    while (1) {
		pad = ReadController(0);

        if ((pad[0]->button & U_CBUTTONS) && (~lastButton & U_CBUTTONS))
        {
            useDebugger(&gGameboy.cpu, &gGameboy.memory);
        }

		lastButton = pad[0]->button;

		OSTime frameTime = lastTime;
		lastTime = osGetTime();

		frameTime = lastTime - frameTime;

		accumulatedTime += OS_CYCLES_TO_USEC(frameTime);

		// time s  1024*1024 cycles/s 60 frames/s

		cstring=str;
		
#if !RUN_TESTS
		lastDrawTime = -osGetTime();

		// gameboy is 60 fps N64 is 30
		// we need to emulator two frames
		// for every one frame rendered
		
		// clearDebugOutput();
		

		loop = MAX_FRAME_SKIP;
		while (accumulatedTime > USECS_PER_FRAME && loop > 0)
		{
			handleInput(&gGameboy, pad[0]);
			emulateFrame(&gGameboy, NULL);
			pad = ReadController(0);
			accumulatedTime -= USECS_PER_FRAME;
			--loop;
		}

		while (accumulatedTime > USECS_PER_FRAME)
		{
			accumulatedTime -= USECS_PER_FRAME;
		}

		handleInput(&gGameboy, pad[0]);
		emulateFrame(&gGameboy, getColorBuffer());
		accumulatedTime -= USECS_PER_FRAME;
		finishAudioFrame(&gGameboy.memory);

		osWritebackDCache(getColorBuffer(), sizeof(u16) * SCREEN_WD*SCREEN_HT);

		lastDrawTime += osGetTime();
		// sprintf(str, "Cycles run %d\nFrame Time %d\nEmu time %d\n%X", 
		// 	gGameboy.cpu.cyclesRun, 
		// 	(int)OS_CYCLES_TO_USEC(frameTime) / 1000, 
		// 	(int)OS_CYCLES_TO_USEC(lastDrawTime) / 1000,
		// 	renderSprites
		// );
		// debugInfo(str);
#endif
		preRenderFrame(0);
		renderDebugLog();
		finishRenderFrame();
    }
}
