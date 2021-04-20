
#ifndef _DEBUG_OUT_H
#define _DEBUG_OUT_H

void debugInfo(char *message);
void clearDebugOutput();
char* getDebugString();

void renderDebugFrame();

extern char debugTMP[100];

#define DEBUG_PRINT_F(formatString, ...) sprintf(debugTMP, formatString, ##__VA_ARGS__); debugInfo(debugTMP)

#endif