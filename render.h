
#ifndef _RENDER_H
#define _RENDER_H

#include <ultra64.h> 
#include "boot.h"

/*
 * Message queues
 */
extern OSMesgQueue
                rdpMessageQ,
                retraceMessageQ;

/*
 * global variables
 */
extern int	rdp_flag;
extern char	*staticSegment;
extern Gfx	*glistp;

/*
 * frame buffer symbols
 */
extern u16	cfb[][SCREEN_WD*SCREEN_HT];	/* RAM address */
extern u16	rsp_cfb[];			/* segment address */
/*
 * buffers for RSP tasks:
 * buffers used by fifo microcode only
 */
extern u64	dram_stack[SP_DRAM_STACK_SIZE64]; /* used for matrix stack */
extern u64	rdp_output[RDP_OUTPUT_LEN];	/* buffer for RDP DL */

extern int		fontcol[4];	/* color for shadowed fonts */

#define	GLIST_LEN	2048

/*
 * Layout of dynamic data.
 *
 * This structure holds the things which change per frame. It is advantageous
 * to keep dynamic data together so that we may selectively write back dirty
 * data cache lines to DRAM prior to processing by the RCP.
 *
 */
typedef struct {
	Gfx	glist[GLIST_LEN];
} Dynamic;

extern Dynamic	dynamic;

/*
 * macros 
 */
#define FONTCOL(r,g,b,a)        {fontcol[0]=r;  \
                                fontcol[1]=g;   \
                                fontcol[2]=b;   \
                                fontcol[3]=a;}
#define FONTCOLM(c)             FONTCOL(c)
#define SHOWFONT(glp,str,x,y)   {                                             \
                font_set_color( 0,0,0,255);                                   \
                font_set_pos( (x)+(1), (y)+(0) );                             \
                font_show_string( glp, str );                                 \
                font_set_pos( (x)+(0), (y)+(1) );                             \
                font_show_string( glp, str );                                 \
                font_set_color( fontcol[0],fontcol[1],fontcol[2],fontcol[3]); \
                font_set_pos( x, y );                                         \
                font_show_string( glp, str );}

void preRenderFrame(int clear);
void renderDebugLog();
void finishRenderFrame();
u16* getColorBuffer();

#endif