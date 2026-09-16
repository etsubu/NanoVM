#include "NanoVM.h"
#include "syscalls.h"

int main(int argc, char* argv[])
{
	if (argc <= 1) {
		printf("Usage NanoVM.exe [FILE]\n");
		return 0;
	}
	NanoVM vm;
	if (NanoVMInitFromFile(&vm, argv[1])) {
		return 1;
	}
	nanovm_syscall table[4] = {0};
	table[3] = &nanovm_syscall_printi;
	if (NanoVMAttachSyscallTable(&vm, table, sizeof(table) / sizeof(table[0]))) {
		printf("Failed to attach standard library to syscall table\n");
		return 1;
	}
	int returnValue = (int)NanoVMRun(&vm);
	NanoVMDestroy(&vm);
	return returnValue;
}
