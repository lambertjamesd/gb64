
#include "cpu_test.h"
#include "graphics_test.h"

int runTests(char* testOutput)
{
    if (
        // !runCPUTests(testOutput) ||
        !testGraphics(testOutput)
    )
    {
        return 0;
    }
    
	sprintf(testOutput, "Tests Passed %X", &runCPU);

    return 1;
}