#include <ultra64.h> 
#include "render.h"
#include "static.h"
#include "src/debug_out.h"

/*
 * Task header.
 */
OSTask          taskHeader =
{
	M_GFXTASK,		/* type of task */
	OS_TASK_DP_WAIT,	/* flags - wait for DP to be available */
	NULL,			/* ucode boot (fill in later) */
	0,			/* ucode boot size (fill in later) */
	NULL,			/* ucode (fill in later) */
	SP_UCODE_SIZE,		/* ucode size */
	NULL,			/* ucode data (fill in later) (to init DMEM) */
	SP_UCODE_DATA_SIZE,	/* ucode data size */
	&dram_stack[0],		/* stack used by ucode */
	SP_DRAM_STACK_SIZE8,	/* size of stack */
	&rdp_output[0],		/* fifo output buffer start */
	&rdp_output[0]+RDP_OUTPUT_LEN,	/* fifo output buffer end */
	NULL,			/* display list pointer (fill in later) */
	0,			/* display list size (ignored) */
	NULL,			/* yield buffer (used if yield will occur) */
	0			/* yield buffer length */
};

/*
 * global variables
 */
Gfx		*glistp;	/* RSP display list pointer */
Dynamic		dynamic;	/* dynamic data */
int		draw_buffer=0;	/* frame buffer being updated (0 or 1) */
int		fontcol[4];	/* color for shadowed fonts */

void renderFrame(int clear)
{
    OSTask      *theadp;
    Dynamic     *dynamicp;
	int x, y;
	x = 18;
	y = 18;
    /*
    * pointers to build the display list.
    */
    theadp = &taskHeader;
    dynamicp = &dynamic;
    glistp = dynamicp->glist;

    /*
    * Tell RCP where each segment is
    */
    gSPSegment(glistp++, 0, 0x0);	/* physical segment */
    gSPSegment(glistp++, STATIC_SEGMENT, 
            OS_K0_TO_PHYSICAL(staticSegment));
    gSPSegment(glistp++, CFB_SEGMENT, 
            OS_K0_TO_PHYSICAL(cfb[draw_buffer]));

    /*
    * Clear color framebuffer.
    */

    gDPSetFillColor(glistp++, GPACK_RGBA5551(
            (int)1,
            (int)1,
            (int)1,
            1) << 16 |
            GPACK_RGBA5551(
            (int)1,
            (int)1,
            (int)1,
            1));
    gSPDisplayList(glistp++, clear ? clearcfb_dl : switchcfb_dl);

    /*
    * Initialize RCP state.
    */
    gSPDisplayList(glistp++, init_dl);

    gDPPipeSync(glistp++);
    gDPSetCycleType(glistp++, G_CYC_1CYCLE);

    /*
    * Draw Text
    */
    {
        /* 
        * neccessary after lines
        */
        gDPSetScissor(glistp++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT);

        font_init( &glistp );
        font_set_transparent( 1 );
        
        font_set_scale( 1.0, 1.0 );
        font_set_win( 200, 1 );

        FONTCOL(55, 255, 155, 255);
    
        char *cstring = getDebugString();

        y = 18;

        while (*cstring)
        {
                SHOWFONT(&glistp,cstring,x,y);

                while (*cstring && *cstring != '\n')
                {
                        ++cstring;
                }

                if (*cstring)
                {
                        ++cstring;
                }

                y += 20;
        }
    }

    font_finish( &glistp );


    gDPFullSync(glistp++);
    gSPEndDisplayList(glistp++);

#ifdef DEBUG
#ifndef __MWERKS__
    assert((glistp - dynamicp->glist) < GLIST_LEN);
#endif
#endif

    /*
    * Build graphics task:
    *
    */
    theadp->t.ucode_boot = (u64 *) rspbootTextStart;
    theadp->t.ucode_boot_size = 
            (u32) rspbootTextEnd - (u32) rspbootTextStart;

    /*
        * RSP output over FIFO to RDP: 
        */
    theadp->t.ucode = (u64 *) gspF3DEX2_fifoTextStart;
    theadp->t.ucode_data = (u64 *) gspF3DEX2_fifoDataStart;

    /*
    * initial display list: 
    */
    theadp->t.data_ptr = (u64 *) dynamicp->glist;
    theadp->t.data_size = 
        (u32) ((glistp - dynamicp->glist) * sizeof(Gfx));

    /*
    * Write back dirty cache lines that need to be read by the RCP.
    */
    osWritebackDCache(&dynamic, sizeof(dynamic));

    /*
    * start up the RSP task
    */
    osSpTaskStart(theadp);

    /*
    * wait for RDP completion 
    */
    (void)osRecvMesg(&rdpMessageQ, NULL, OS_MESG_BLOCK);

    /*
    * setup to swap buffers 
    */
    osViSwapBuffer(cfb[draw_buffer]);

    /*
    * Make sure there isn't an old retrace in queue 
    * (assumes queue has a depth of 1) 
    */
    while (!MQ_IS_EMPTY(&retraceMessageQ)) {
        (void) osRecvMesg(&retraceMessageQ, NULL, OS_MESG_NOBLOCK);
    }

    /*
    * Wait for Vertical retrace to finish swap buffers 
    */
    (void) osRecvMesg(&retraceMessageQ, NULL, OS_MESG_BLOCK);

    draw_buffer ^= 1;
}

u16* getColorBuffer()
{
    return cfb[draw_buffer];
}