
#ifndef _GB_MEMORY_H_
#define _GB_MEMORY_H_

extern unsigned int _gMemoryStart;

extern char* gCurrentHeapPosition;

void *malloc(unsigned int size);
int getFreeBytes();

#endif