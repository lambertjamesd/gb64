
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
 * File:	boot.h
 * Create Date:	Thu Dec 14 11:40:21 PST 1995
 *
 */

#define	STATIC_SEGMENT		1
#define	CFB_SEGMENT		2
#define	DYNAMIC_SEGMENT		3
#define	TEXTURE_SEGMENT		4

#define	SCREEN_HT	240
#define	SCREEN_WD	320

/* this stack size is in bytes, and is a lot larger
 * than this program needs.
 */
#define	STACKSIZE	0x2000
#define RDP_OUTPUT_LEN	(4096*16)
/*
 * this number (the depth of the message queue) needs to be equal
 *   to the maximum number of possible overlapping PI requests.
 *   For this app, 1 or 2 is probably plenty, other apps might
 *   require a lot more.
 */
#define NUM_PI_MSGS     8
