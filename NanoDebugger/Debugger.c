#include "NanoDebugger.h"

int main(int argc, char *argv[])
{
	if (argc <= 1) {
		printf("Usage NanoDebugger.exe [FILE]\n");
		fflush(stdout);
	}
	NanoDebugger debugger;
	NanoDebuggerInit(&debugger, argv[1]);
	NanoDebuggerDebug(&debugger);
	NanoDebuggerDestroy(&debugger);
	return 0;
}
