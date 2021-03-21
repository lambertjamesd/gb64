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
#include "src/faulthandler.h"

#include "debugger/debugger.h"

#define RUN_TESTS 0

char gTmpBuffer[512];

/*
 * This is the main routine of the app.
 */
void
game(void)
{
    OSContPad	**pad;
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
    char	str[200];
	runTests(str);
	debugInfo(str);
#endif

	initGameboy(&gGameboy, &gGBRom);
	
    initAudio(&gAudioState, 22500, 30);

	initDebugMenu(&gDebugMenu, &gGameboy.cpu, &gGameboy.memory);

	initMainMenu(&gMainMenu);

	int frames = 0;

    /*
     * Main game loop
     */
    /*osSyncPrintf("Use the L button and crosshair for menu options.\n");*/
    while (1) {
		lastButton = ReadLastButton(0);
		pad = ReadController(0);

        if ((pad[0]->button & L_CBUTTONS) && (~lastButton & L_CBUTTONS) && (pad[0]->button & R_TRIG) && (pad[0]->button & L_TRIG))
        {
            useDebugger(&gGameboy.cpu, &gGameboy.memory);
        }

		u32 currentTime = osGetCount();
		u32 frameTime = currentTime - lastTime;
		lastTime = currentTime;

		// time s  1024*1024 cycles/s 60 frames/s
		
#if !RUN_TESTS
		lastDrawTime = -osGetCount();

		// gameboy is 60 fps N64 is 30
		// we need to emulator two frames
		// for every one frame rendered
		
		// clearDebugOutput();

		if (!isMainMenuOpen(&gMainMenu))
		{		
			accumulatedTime += frameTime;

			if (lastButton & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.fastForward))
			{
				accumulatedTime += CPU_TICKS_PER_FRAME * 6;
			}

			loop = MAX_FRAME_SKIP;
			// frames = 0;
			while (accumulatedTime > CPU_TICKS_PER_FRAME && loop > 0)
			{
				handleGameboyInput(&gGameboy, pad[0]);
				emulateFrame(&gGameboy, 0);
				pad = ReadController(0);
				accumulatedTime -= CPU_TICKS_PER_FRAME;
				--loop;
				++frames;
			}

			while (accumulatedTime > CPU_TICKS_PER_FRAME)
			{
				accumulatedTime -= CPU_TICKS_PER_FRAME;
			}
			
			handleGameboyInput(&gGameboy, pad[0]);
			emulateFrame(&gGameboy, getColorBuffer());
			accumulatedTime -= CPU_TICKS_PER_FRAME;
			finishAudioFrame(&gGameboy.memory);

			++frames;
		}

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

		renderDebugLog();

		updateMainMenu(&gMainMenu, pad[0]);
		renderMainMenu(&gMainMenu);

		finishRenderFrame();

		faultHandlerHeartbeat();
    }
}
