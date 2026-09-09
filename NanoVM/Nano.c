#include "NanoVM.h"

int main(int argc, char* argv[])
{
	if (argc <= 1) {
		printf("Usage NanoVM.exe [FILE]\n");
		return 0;
	}
	NanoVM vm;
	NanoVMInitFromFile(&vm, argv[1]);
	int returnValue = (int)NanoVMRun(&vm);
	NanoVMDestroy(&vm);
	return returnValue;
}
