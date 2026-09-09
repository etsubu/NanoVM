#include "NanoDebugger.h"
#include <inttypes.h>

#define INSTRUCTION_LENGTH 128

static size_t findBreakpoint(const NanoDebugger* debugger, uint64_t offset) {
	for (size_t i = 0; i < debugger->breakpointCount; i++) {
		if (debugger->breakpoints[i] == offset) {
			return i;
		}
	}
	return debugger->breakpointCount;
}

static void insertBreakpoint(NanoDebugger* debugger, uint64_t offset) {
	if (findBreakpoint(debugger, offset) != debugger->breakpointCount) {
		return;
	}
	if (debugger->breakpointCount == debugger->breakpointCapacity) {
		size_t capacity = (debugger->breakpointCapacity == 0) ? 8 : debugger->breakpointCapacity * 2;
		uint64_t* breakpoints = (uint64_t*)realloc(debugger->breakpoints, capacity * sizeof(uint64_t));
		if (breakpoints == NULL) {
			return;
		}
		debugger->breakpoints = breakpoints;
		debugger->breakpointCapacity = capacity;
	}
	debugger->breakpoints[debugger->breakpointCount] = offset;
	debugger->breakpointCount++;
}

static void eraseBreakpoint(NanoDebugger* debugger, size_t index) {
	memmove(&debugger->breakpoints[index], &debugger->breakpoints[index + 1], (debugger->breakpointCount - index - 1) * sizeof(uint64_t));
	debugger->breakpointCount--;
}

static bool disassembleInstruction(NanoDebugger* debugger, char* instruction, size_t length) {
	Instruction ins;
	if (!NanoVMFetch(&debugger->vm, &ins)) {
		return false;
	}
	const char* opcode = instructionStr[ins.opcode];
	if (ins.opcode == Halt || ins.opcode == Ret) {
		snprintf(instruction, length, "%s", opcode);
		return true;
	}
	if (ins.opcode == Jg || ins.opcode == Js || ins.opcode == Jnz || ins.opcode == Jz ||
		ins.opcode == Jmp || ins.opcode == Push || ins.opcode == Pop || ins.opcode == Call ||
		ins.opcode == Dec || ins.opcode == Inc || ins.opcode == Printc || ins.opcode == Printi ||
		ins.opcode == Prints) {
		if (ins.srcType == Reg) {
			snprintf(instruction, length, "%s%s%d", opcode, (ins.isSrcMem) ? " @reg" : " reg", ins.srcReg);
		}
		else {
			snprintf(instruction, length, "%s%s%" PRIu64, opcode, (ins.isSrcMem) ? " @" : " ", ins.immediate);
		}
	}
	else {
		if (ins.srcType == Reg) {
			snprintf(instruction, length, "%s%s%d, %s%d", opcode, (ins.isDstMem) ? " @reg" : " reg", ins.dstReg,
				(ins.isSrcMem) ? " @reg" : "reg", ins.srcReg);
		}
		else {
			snprintf(instruction, length, "%s%s%d, %s%" PRIu64, opcode, (ins.isDstMem) ? " @reg" : " reg", ins.dstReg,
				(ins.isSrcMem) ? "@" : "", ins.immediate);
		}
	}
	return true;
}

static void printStack(NanoDebugger* debugger) {
	int counter = 0;
	unsigned char *p = (debugger->vm.cpu.stackBase);
	uint64_t size = (debugger->vm.cpu.registers[esp] + debugger->vm.cpu.codeBase) - debugger->vm.cpu.stackBase;
	printf("\nStack size: %" PRIu64 "\n", size);
	for (int i = 0; i < size; i++) {
		if (counter == 7) {
			counter = 0;
			printf("%02X | %c %c %c %c %c %c %c %c\n", p[i], p[i - 7], p[i - 6], p[i - 5], p[i - 4], p[i - 3], p[i - 2], p[i - 1], p[i]);
			continue;
		}
		else {
			printf("%02X ", p[i]);
		}
		counter++;
	}
	if (counter) {
		printf("|  ");
		for (int i = counter; i > 0; i--) {
			printf("%c  ", p[size - i]);
		}
		printf("\n");
	}
	printf("\n");
}

static bool handleInteractive(NanoDebugger* debugger) {
	int value = 0;
	do {
		char instruction[INSTRUCTION_LENGTH];
		if (!disassembleInstruction(debugger, instruction, sizeof(instruction))) {
			printf("Failed to fetch instruction: IP out of bounds! IP: %" PRIu64 "\n", debugger->vm.cpu.registers[ip]);
			fflush(stdout);
			return false;
		}
		printf("%" PRIu64 ". %s\n", debugger->vm.cpu.registers[ip], instruction);
		fflush(stdout);
		printf("> ");
		value = getchar();
		printf("\b\b");
		if (value == 'h') {
			printf("\n(s)tack\nr(e)gisters\n(b)reakpoint\n(r)un\n(c)lean breakpoint\n(q)uit\n");
			fflush(stdout);
		}
		else if (value == 'e') {
			printf("\nRegisters:\n");
			for (int i = 0; i < 8; i++) {
				printf("reg%d: %" PRIu64 "\n", i, debugger->vm.cpu.registers[i]);
				fflush(stdout);
			}
		}
		else if (value == 'r') {
			debugger->run = true;
			return true;
		}
		else if (value == 'b') {
			printf("Breakpoint where (offset): ");
			int offset;
			scanf("%d", &offset);
			insertBreakpoint(debugger, offset);
		}
		else if (value == 'c') {
			size_t a = findBreakpoint(debugger, debugger->vm.cpu.registers[ip]);
			if (a == debugger->breakpointCount) {
				printf("No breakpoint was placed here!\n");
				fflush(stdout);
			}
			else {
				eraseBreakpoint(debugger, a);
				printf("Breakpoint removed!\n");
				fflush(stdout);
			}
		}
		else if (value == 's') {
			printStack(debugger);
		}
		else if (value == 'q') {
			return false;
		}
	} while (value != 13);
	return true;
}

void NanoDebuggerInit(NanoDebugger* debugger, const char* file) {
	NanoVMInitFromFile(&debugger->vm, file);
	debugger->breakpoints = NULL;
	debugger->breakpointCount = 0;
	debugger->breakpointCapacity = 0;
	debugger->run = false;
}

void NanoDebuggerInitFromMemory(NanoDebugger* debugger, unsigned char* bytecode, uint64_t size) {
	NanoVMInit(&debugger->vm, bytecode, size);
	debugger->breakpoints = NULL;
	debugger->breakpointCount = 0;
	debugger->breakpointCapacity = 0;
	debugger->run = false;
}

void NanoDebuggerDestroy(NanoDebugger* debugger) {
	free(debugger->breakpoints);
	NanoVMDestroy(&debugger->vm);
}

bool NanoDebuggerDebug(NanoDebugger* debugger) {
	debugger->run = false;
	while (debugger->vm.cpu.registers[ip] < debugger->vm.cpu.bytecodeSize) {
		Instruction inst;
		if (NanoVMFetch(&debugger->vm, &inst)) {
			if (findBreakpoint(debugger, debugger->vm.cpu.registers[ip]) != debugger->breakpointCount) {
				printf("Breakpoint triggered! %" PRIu64 "\n", debugger->vm.cpu.registers[ip]);
				fflush(stdout);
				debugger->run = false;
				handleInteractive(debugger);
			}
			else if (!debugger->run) {
				handleInteractive(debugger);
			}
			if (inst.opcode == Halt) {
				printf("VM halted!\n");
				fflush(stdout);
				handleInteractive(debugger);
				break;
			}
			if (!NanoVMExecute(&debugger->vm, &inst)) {
				switch (debugger->vm.errorFlag) {
				case MEMORY_ACCESS:
					printf("Tried to read/write memory outside of VM!\n");
					fflush(stdout);
					break;
				default:
					printf("Unknown error!\n");
					fflush(stdout);
				}
				return false;
			}
		}
		else {
			printf("Invalid instruction!\n");
			fflush(stdout);
			return false;
		}
	}
	printf("VM exited with return code: %" PRIu64 "\n", debugger->vm.cpu.registers[Reg0]);
	fflush(stdout);
	handleInteractive(debugger);
	return true;
}
