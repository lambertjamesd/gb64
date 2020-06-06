#include "debug_out.h"

#define DEBUG_BUFFER_SIZE 1024
#define MAX_DEBUG_LINES 8

static char debugBuffer[DEBUG_BUFFER_SIZE];
static int currentWritePosition = 0;
static int currentSourcePosition = 0;
static int visibleDebugLineCount;

void wrapBuffer()
{
    int targetIndex = 0;
    
    while (currentSourcePosition < currentWritePosition)
    {
        debugBuffer[targetIndex++] = debugBuffer[currentSourcePosition++];
    }

    debugBuffer[targetIndex] = 0;
    currentSourcePosition = 0;
    currentWritePosition = targetIndex;
}

void removeOldestLine()
{
    while (currentSourcePosition < currentWritePosition)
    {
        if (debugBuffer[currentSourcePosition] == '\n')
        {
            --visibleDebugLineCount;
            ++currentSourcePosition;
            break;
        }
        else
        {
            ++currentSourcePosition;
        }
    }
}

void debugInfo(char *message)
{
    while (*message)
    {
        if (*message == '\n')
        {
            ++visibleDebugLineCount;

            if (visibleDebugLineCount > MAX_DEBUG_LINES)
            {
                removeOldestLine();
            }
        }

        debugBuffer[currentWritePosition] = *message;

        ++message;
        ++currentWritePosition;

        if (currentWritePosition == DEBUG_BUFFER_SIZE)
        {
            wrapBuffer();
        }
    }

    debugBuffer[currentWritePosition] = 0;
}

void clearDebugOutput()
{
    debugBuffer[0] = 0;
    currentSourcePosition = 0;
    currentWritePosition = 0;
    visibleDebugLineCount = 0;
}

char* getDebugString()
{
    return debugBuffer + currentSourcePosition;
}