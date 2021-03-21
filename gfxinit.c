
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
#include "render.h"

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
 * initialize the RCP state
 */
Gfx init_dl[] = {
    /*
     * init RSP
     */
    gsSPViewport(&vp),
    gsSPClearGeometryMode(G_SHADE | G_SHADING_SMOOTH | G_CULL_BOTH |
			  G_FOG | G_LIGHTING | G_TEXTURE_GEN |
			  G_TEXTURE_GEN_LINEAR | G_LOD),
    gsSPTexture(0, 0, 0, 0, G_OFF),
    gsSPSetGeometryMode(G_SHADE | G_SHADING_SMOOTH),
    gsSPClipRatio(FRUSTRATIO_2),

    /*
     * init RDP
     */
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsDPSetScissor(G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT),
    gsDPSetTextureLOD(G_TL_TILE),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetTextureDetail(G_TD_CLAMP),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsDPSetCombineKey(G_CK_NONE),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
    gsDPSetColorDither(G_CD_BAYER),
    gsDPSetAlphaDither(G_AD_NOISE),
    gsDPPipeSync(),
    gsSPEndDisplayList(),
};

/*
 * clear the color frame buffer
 */
Gfx clearcfb_dl[] = {
    gsDPSetCycleType(G_CYC_FILL),
#if 0
    gsDPSetFillColor(GPACK_RGBA5551(64,64,255,1) << 16 | 
		     GPACK_RGBA5551(64,64,255,1)),
#endif
    // gsDPFillRectangle(0, 0, SCREEN_WD-1, SCREEN_HT-1),
    gsSPEndDisplayList(),
};

