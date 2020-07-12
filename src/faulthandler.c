
#include <ultra64.h>
#include <os_internal.h>
#include "faulthandler.h"
#include "render.h"
#include "debug_out.h"

#define FAULT_STACK_SIZE     0x200

static OSThread faultThread;
static u64      faultThreadStack[FAULT_STACK_SIZE / sizeof(u64)];

static void     faultHandlerProc(void *);

void installFaultHandler()
{
	/*
	 * Create main thread
	 */
	osCreateThread(&faultThread, 4, faultHandlerProc, NULL,
		   faultThreadStack + FAULT_STACK_SIZE / sizeof(u64), 10);

	osStartThread(&faultThread);
}

static void faultHandlerProc(void* arg)
{
    while (1) {
        /* 
         * This routine returns the next faulted thread from the active
         * thread list. It uses the thread argument as the starting point of
         * the search: if NULL, starts from the beginning of the list.
         * The routine returns NULL if it can't find any faulted thread.
         */
        OSThread *curr = __osGetNextFaultedThread(NULL);
        if (curr) {
            DEBUG_PRINT_F("Main thread faulted:\n");
            DEBUG_PRINT_F("  pc=0x%08x\n", curr->context.pc);
            DEBUG_PRINT_F("  badvaddr=0x%08x\n", curr->context.badvaddr);
            DEBUG_PRINT_F("  ra=0x%x\n", curr->context.ra);
            DEBUG_PRINT_F("  cause=0x%x\n", curr->context.cause);
            break;
        } else {
            osYieldThread();
        }
    }

    while (1)
    {
		preRenderFrame();
		renderDebugLog();
		finishRenderFrame();
    }
}