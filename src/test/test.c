
#include "cpu_test.h"

int runTests(char* testOutput)
{
    if (
        !runCPUTests(testOutput)
    )
    {
        return 0;
    }
    
	sprintf(testOutput, "Tests Passed %X", &runCPU);

    return 1;
}