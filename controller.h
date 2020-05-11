
#ifndef _CONTROLLER_INCLD
#define _CONTROLLER_INCLD

extern int 	  numControllers;

extern int 	  initControllers(int maxcontrollers);
extern OSContPad  **ReadController(int oneshot);


#endif /* _CONTROLLER_INCLD */
