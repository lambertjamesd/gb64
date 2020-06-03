
#include "cpu_test.h"
#include "graphics_test.h"

int runTests(char* testOutput)
{
    return 
        runCPUTests(testOutput) ||
        testGraphics(testOutput) ||
    1;
}