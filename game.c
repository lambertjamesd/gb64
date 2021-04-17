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

#include "memory.h"
#include "game.h"
#include "static.h"
#include "controller.h"
#include "src/gameboy.h"
#include "src/graphics.h"
#include "src/test/test.h"
#include "src/debug_out.h"
#include "render.h"
#include "src/debugger.h"
#include "src/mainmenu.h"
#include "src/faulthandler.h"
#include "src/rspppu.h"
#include "src/spritefont.h"
#include "src/sprite.h"
#include "tex/textures.h"

#include "debugger/debugger.h"

#define RUN_TESTS 0

extern void test_malloc();

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

	// test_malloc();

	initGameboy(&gGameboy, &gGBRom);
	
    initAudio(&gAudioState, 22500, 30);

	initDebugMenu(&gDebugMenu, &gGameboy.cpu, &gGameboy.memory);

	initMainMenu(&gMainMenu);

	initGBFont();

	// addBreakpoint(&gGameboy.memory, 0x100, BreakpointTypeUser);

	int frames = 0;

	setLayerGraphics(SPRITE_BORDER_LAYER, gUseGUIItems);
	setLayerGraphics(SPRITE_CBUTTONS_LAYER, gUseCButtons);
	setLayerGraphics(SPRITE_TRIGGERS_LAYER, gUseTriggers);
	setLayerGraphics(SPRITE_DPAD_LAYER, gUseDPad);
	setLayerGraphics(SPRITE_FACE_LAYER, gUseFaceButtons);
    setLayerGraphics(SPRITE_FONT_LAYER, gUseFontTexture);

    /*
     * Main game loop
     */
    /*osSyncPrintf("Use the L button and crosshair for menu options.\n");*/
    while (1) {
    	initSprites();
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

		OSTime startTime = osGetTime();

		if (isMainMenuOpen(&gMainMenu))
		{
			rerenderLastFrame(&gGameboy.settings.graphics, getColorBuffer());
		}
		else
		{		
			accumulatedTime += frameTime;

			if (lastButton & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.fastForward))
			{
				accumulatedTime += CPU_TICKS_PER_FRAME * 6;
			}

			loop = MAX_FRAME_SKIP;
			frames = 0;
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

		OSTime totalTime = osGetTime() - startTime;

		lastDrawTime += osGetCount();
		// clearDebugOutput();
		// sprintf(gTmpBuffer, "CPU M0 Wait %d\nPPU M2 Wait %d\nPPU M3 Wait %d\nFrames %d\nFrame Time %d\n", 
		// 	gCyclesWaitingForMode0,
		// 	gPPUPerformance.mode2StallCount,
		// 	gPPUPerformance.mode3StallCount,
		// 	frames,
		// 	(int)OS_CYCLES_TO_USEC(totalTime) / 1000 
		// );
		// debugInfo(gTmpBuffer);
#endif
		updateMainMenu(&gMainMenu, pad[0]);
		renderMainMenu(&gMainMenu);

		renderFrame(0);

		faultHandlerHeartbeat();
    }
}
