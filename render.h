
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
extern u16*	cfb;	/* RAM address */
/*
 * buffers for RSP tasks:
 * buffers used by fifo microcode only
 */
extern u64	dram_stack[SP_DRAM_STACK_SIZE64]; /* used for matrix stack */
extern u64	rdp_output[RDP_OUTPUT_LEN];	/* buffer for RDP DL */

extern int		fontcol[4];	/* color for shadowed fonts */

#define PACK16BIT(r, g, b) ((u16)(r) << 8 & 0xF800 | (u16)(g) << 3 & 0x07C0 | (u16)(b) >> 2 & 0x003E | 0x01)
#define COL24TO16(rgb24) ((rgb24) >> 8 & 0xF800 | (rgb24) >> 5 & 0x07C0 | (rgb24) >> 2 & 0x003E | 0x01)
#define GET_R(rgb16) ((rgb16) >> 8 & 0xF8)
#define GET_G(rgb16) ((rgb16) >> 3 & 0xF8)
#define GET_B(rgb16) ((rgb16) << 2 & 0xF8)

#define	GLIST_LEN	2048

#define BUFFER_COUNT 2

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

void preRenderFrame(int clear);
void renderDebugLog();
void finishRenderFrame();
u16* getColorBuffer();
void* initColorBuffers(void* memoryEnd);

#endif