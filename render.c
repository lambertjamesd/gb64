#include "render.h"
#include "static.h"
#include "src/debug_out.h"
#include "memory.h"

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
Dynamic     *dynamicp;
u16* cfb;

void preRenderFrame()
{
    /*
    * pointers to build the display list.
    */
    dynamicp = &dynamic;
    glistp = dynamicp->glist;

    /*
    * Tell RCP where each segment is
    */
    gSPSegment(glistp++, 0, 0x0);	/* physical segment */
    gSPSegment(glistp++, STATIC_SEGMENT, OS_K0_TO_PHYSICAL(staticSegment));

    gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD, OS_K0_TO_PHYSICAL(getColorBuffer()));

    /*
    * Initialize RCP state.
    */
    gSPDisplayList(glistp++, init_dl);

    gDPPipeSync(glistp++);
    gDPSetCycleType(glistp++, G_CYC_1CYCLE);
}

void renderDebugLog()
{
	int x, y;
	x = 18;
    y = 18;
    
    /* 
    * neccessary after lines
    */
    gDPSetScissor(glistp++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT);

    font_init( &glistp );
    
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

void finishRenderFrame()
{
    OSTask      *theadp;
    Dynamic     *dynamicp;

    theadp = &taskHeader;
    dynamicp = &dynamic;

    font_finish( &glistp );

    glistp = dynamicp->glist;

    gDPFullSync(glistp++);
    gSPEndDisplayList(glistp++);

    theadp->t.ucode_boot = (u64 *) rspbootTextStart;
    theadp->t.ucode_boot_size = 
            (u32) rspbootTextEnd - (u32) rspbootTextStart;

    theadp->t.ucode = (u64 *) gspF3DEX2_fifoTextStart;
    theadp->t.ucode_data = (u64 *) gspF3DEX2_fifoDataStart;

    theadp->t.data_ptr = (u64 *) dynamicp->glist;
    theadp->t.data_size = 
        (u32) ((glistp - dynamicp->glist) * sizeof(Gfx));

    osWritebackDCache(&dynamic, sizeof(dynamic));
    osSpTaskStart(theadp);
    (void)osRecvMesg(&rdpMessageQ, NULL, OS_MESG_BLOCK);

    osViSwapBuffer(getColorBuffer());
    while (!MQ_IS_EMPTY(&retraceMessageQ)) {
        (void) osRecvMesg(&retraceMessageQ, NULL, OS_MESG_NOBLOCK);
    }
    (void) osRecvMesg(&retraceMessageQ, NULL, OS_MESG_BLOCK);

    draw_buffer = (draw_buffer + 1) % BUFFER_COUNT;
}

u16* getColorBuffer()
{
    return cfb + draw_buffer * (SCREEN_WD * SCREEN_HT);
}

void* initColorBuffers(void* memoryEnd)
{
    cfb = (u16*)memoryEnd - BUFFER_COUNT * SCREEN_WD * SCREEN_HT;
    zeroMemory(cfb, sizeof(u16) * BUFFER_COUNT * SCREEN_WD * SCREEN_HT);
    return cfb;
}