
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

/*
 * File:	gfxinit.c
 *
 * This file holds display list segments that are 'static' data.
 *
 */

#include <ultra64.h>
#include "boot.h"
#include "game.h"

/*
 * Remember, viewport structures have 2 bits of fraction in them.
 */
static Vp vp = {
	SCREEN_WD*2, SCREEN_HT*2, G_MAXZ/2, 0,	/* scale */
	SCREEN_WD*2, SCREEN_HT*2, G_MAXZ/2, 0,	/* translate */
};
static Vp vp1 = {
	SCREEN_WD*1, SCREEN_HT*1, G_MAXZ/2, 0,	/* scale */
	SCREEN_WD*1, SCREEN_HT*1, G_MAXZ/2, 0,	/* translate */
};
static Vp vp2 = {
	SCREEN_WD*1, SCREEN_HT*1, G_MAXZ/2, 0,	/* scale */
	SCREEN_WD*3, SCREEN_HT*1, G_MAXZ/2, 0,	/* translate */
};
static Vp vp3 = {
	SCREEN_WD*1, SCREEN_HT*1, G_MAXZ/2, 0,	/* scale */
	SCREEN_WD*1, SCREEN_HT*3, G_MAXZ/2, 0,	/* translate */
};
static Vp vp4 = {
	SCREEN_WD*1, SCREEN_HT*1, G_MAXZ/2, 0,	/* scale */
	SCREEN_WD*3, SCREEN_HT*3, G_MAXZ/2, 0,	/* translate */
};


/*
 * clear the z (depth) frame buffer
 */
Gfx clearzbuffer_dl[] = {
    gsDPSetScissor(G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT),
    gsDPSetDepthImage(OS_K0_TO_PHYSICAL(zbuffer)),
    gsDPSetCycleType(G_CYC_FILL),
    gsDPSetColorImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD, 
					OS_K0_TO_PHYSICAL(zbuffer)),
    gsDPSetFillColor(GPACK_ZDZ(G_MAXFBZ,0) << 16 | GPACK_ZDZ(G_MAXFBZ,0)),
    gsDPFillRectangle(0, 0, SCREEN_WD-1, SCREEN_HT-1),
    gsSPEndDisplayList(),
};

/*
 * clear the color frame buffer
 */
Gfx clearcfb_dl[] = {
    gsDPSetCycleType(G_CYC_FILL),
    gsDPSetColorImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD, rsp_cfb),
    gsDPSetFillColor(GPACK_RGBA5551(64,64,255,1) << 16 | 
		     GPACK_RGBA5551(64,64,255,1)),
    gsDPFillRectangle(0, 0, SCREEN_WD-1, SCREEN_HT-1),
    gsSPEndDisplayList(),
};

