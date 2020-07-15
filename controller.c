
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

#include <ultra64.h>

/* app specific includes */
#include "boot.h"

OSMesgQueue	controllerMsgQ;
OSMesg		controllerMsgBuf;

OSContStatus	statusdata[MAXCONTROLLERS];
OSContPad	dummycontrollerdata = { 0, 0, 0, 0 };
OSContPad	controllerdata[MAXCONTROLLERS];
OSContPad	*validcontrollerdata[MAXCONTROLLERS];
int		activeControllers[MAXCONTROLLERS];
int		numControllers=0;
u16	    gLastButton[MAXCONTROLLERS];

/*
 * Return how many controllers are connected
 * if there are more than <maxcontrollers> connected, return maxcontrollers
 * (ie specify how many controllers you want with maxcontrollers, and
 *  the return result is the number of controllers actually hooked up)
 */
int initControllers( int maxcontrollers )
{
    int             i;
    u8              pattern;
    OSMesgQueue     serialMsgQ;
    OSMesg          serialMsg;

    osCreateMesgQueue(&serialMsgQ, &serialMsg, 1);
    osSetEventMesg(OS_EVENT_SI, &serialMsgQ, (OSMesg)1);

    osContInit(&serialMsgQ, &pattern, &statusdata[0]);

    osCreateMesgQueue(&controllerMsgQ, &controllerMsgBuf, 1);
    osSetEventMesg(OS_EVENT_SI, &controllerMsgQ, (OSMesg)0);

    for (i = 0; i < MAXCONTROLLERS; i++)
	validcontrollerdata[i] = &dummycontrollerdata;

    for (i = 0; i < MAXCONTROLLERS; i++) {
        if ((pattern & (1<<i)) &&
                !(statusdata[i].errno & CONT_NO_RESPONSE_ERROR)) {
	    validcontrollerdata[numControllers++] = &controllerdata[i];
	    if (numControllers == maxcontrollers) return numControllers;
	}
    }
    osContStartReadData(&controllerMsgQ);
    return numControllers;
}

/*
 * return pointer to controller data for each connected controller
 * oneshot = which buttons to treat as one-shots ("fire" buttons)
 * oneshot is any of the button macros (eg CONT_B, CONT_LEFT) ored together)
 */
OSContPad **ReadController(int oneshot)
{
    int 	i;
    u16		button;

    if (osRecvMesg(&controllerMsgQ, NULL, OS_MESG_NOBLOCK) != -1) {
    	osContGetReadData(controllerdata);
	osContStartReadData(&controllerMsgQ);
    }

    for (i=0; i<numControllers; i++) {
	button = validcontrollerdata[i]->button;
	validcontrollerdata[i]->button = 
		button & (~gLastButton[i] | ~oneshot);
	gLastButton[i]=button;
    }

    return validcontrollerdata;
}

extern u16 ReadLastButton(int index)
{
    return gLastButton[index];
}
