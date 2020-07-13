
#ifndef _FAULT_HANDLER_H
#define _FAULT_HANDLER_H

#include <ultra64.h>

void installFaultHandler(OSThread *targetThread);
void faultHandlerHeartbeat();

#endif