
#ifndef _CONTROLLER_INCLD
#define _CONTROLLER_INCLD

extern int 	  numControllers;

extern int 	  initControllers(int maxcontrollers);
extern OSContPad  **ReadController(int oneshot);
extern u16 ReadLastButton(int index);


#endif /* _CONTROLLER_INCLD */
