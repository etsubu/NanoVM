#include "NanoDebugger.h"
#include <inttypes.h>

#define INSTRUCTION_LENGTH 128

static const char* instructionStr[] = {
	"mov", "add", "sub", "and", "or", "xor", "shr", "shl", "mul", "div", "mod", "cmp",
	"jz", "jnz", "jg", "js", "jmp", "not", "call", "push", "pop", "ret", "halt",
	"load", "load8", "load8u", "store", "store8", "syscall", "printi", "alloc", "free",
	"inc", "dec"
};

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
	if (NanoVMFetch(&debugger->vm, &ins)) {
		return false;
	}

	if (ins.opcode >= sizeof(instructionStr) / sizeof(instructionStr[0])) {
		snprintf(instruction, length, "db 0x%02X", ins.opcode);
		return true;
	}
	const char* opcode = instructionStr[ins.opcode];
	if (ins.opcode == Halt || ins.opcode == Ret) {
		snprintf(instruction, length, "%s", opcode);
		return true;
	}
	if (ins.opcode == Jg || ins.opcode == Js || ins.opcode == Jnz || ins.opcode == Jz ||
		ins.opcode == Jmp || ins.opcode == Push || ins.opcode == Pop || ins.opcode == Call ||
		ins.opcode == Not || ins.opcode == Printi || ins.opcode == Free ||
		ins.opcode == Inc || ins.opcode == Dec) {
		if (ins.srcType == Reg) {
			snprintf(instruction, length, "%s reg%d", opcode, ins.srcReg);
		}
		else {
			snprintf(instruction, length, "%s %" PRId64, opcode, ins.immediate);
		}
	}
	else {
		if (ins.srcType == Reg) {
			snprintf(instruction, length, "%s reg%d, reg%d", opcode, ins.dstReg, ins.srcReg);
		}
		else {
			snprintf(instruction, length, "%s reg%d, %" PRId64, opcode, ins.dstReg, ins.immediate);
		}
	}
	return true;
}

static void printStack(NanoDebugger* debugger) {
	int counter = 0;
	unsigned char *p = (debugger->vm.cpu.stackBase);
	uint64_t size = (uint64_t)debugger->vm.cpu.registers[sp] - debugger->vm.cpu.codeSize;
	printf("\nStack size: %" PRIu64 "\n", size);
	for (uint64_t i = 0; i < size; i++) {
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
			printf("%c  ", p[size - (uint64_t)i]);
		}
		printf("\n");
	}
	printf("\n");
}

static bool handleInteractive(NanoDebugger* debugger) {
	char line[INSTRUCTION_LENGTH];
	int value = 0;
	do {
		char instruction[INSTRUCTION_LENGTH];
		if (!disassembleInstruction(debugger, instruction, sizeof(instruction))) {
			printf("Failed to fetch instruction: IP out of bounds! IP: %" PRId64 "\n", debugger->vm.cpu.registers[ip]);
			fflush(stdout);
			return false;
		}
		printf("%" PRId64 ". %s\n", debugger->vm.cpu.registers[ip], instruction);
		printf("> ");
		fflush(stdout);
		if (!fgets(line, sizeof(line), stdin)) {
			return false;
		}
		value = line[0];
		if (value == 'h') {
			printf("\n(s)tack\nr(e)gisters\n(b)reakpoint\n(r)un\n(c)lean breakpoint\n(q)uit\n");
			fflush(stdout);
		}
		else if (value == 'e') {
			printf("\nRegisters:\n");
			for (int i = 0; i < 8; i++) {
				printf("reg%d: %" PRId64 "\n", i, debugger->vm.cpu.registers[i]);
			}
			fflush(stdout);
		}
		else if (value == 'r') {
			debugger->run = true;
			return true;
		}
		else if (value == 'b') {
			printf("Breakpoint where (offset): ");
			fflush(stdout);
			if (fgets(line, sizeof(line), stdin)) {
				long offset = strtol(line, NULL, 0);
				if (offset >= 0) {
					insertBreakpoint(debugger, (uint64_t)offset);
				}
			}
		}
		else if (value == 'c') {
			size_t a = findBreakpoint(debugger, (uint64_t)debugger->vm.cpu.registers[ip]);
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
	} while (value != '\n');
	return true;
}

bool NanoDebuggerInit(NanoDebugger* debugger, const char* file) {
	debugger->breakpoints = NULL;
	debugger->breakpointCount = 0;
	debugger->breakpointCapacity = 0;
	debugger->run = false;
	return NanoVMInitFromFile(&debugger->vm, file) == 0;
}

bool NanoDebuggerInitFromMemory(NanoDebugger* debugger, unsigned char* bytecode, uint64_t size) {
	debugger->breakpoints = NULL;
	debugger->breakpointCount = 0;
	debugger->breakpointCapacity = 0;
	debugger->run = false;
	return NanoVMInit(&debugger->vm, bytecode, size) == 0;
}

void NanoDebuggerDestroy(NanoDebugger* debugger) {
	free(debugger->breakpoints);
	NanoVMDestroy(&debugger->vm);
}

bool NanoDebuggerDebug(NanoDebugger* debugger) {
	debugger->run = false;
	while ((uint64_t)debugger->vm.cpu.registers[ip] < debugger->vm.cpu.codeSize) {
		Instruction inst;
		if (NanoVMFetch(&debugger->vm, &inst)) {
			printf("Invalid instruction!\n");
			fflush(stdout);
			return false;
		}
		if (findBreakpoint(debugger, (uint64_t)debugger->vm.cpu.registers[ip]) != debugger->breakpointCount) {
			printf("Breakpoint triggered! %" PRId64 "\n", debugger->vm.cpu.registers[ip]);
			fflush(stdout);
			debugger->run = false;
			if (!handleInteractive(debugger)) {
				return false;
			}
		}
		else if (!debugger->run) {
			if (!handleInteractive(debugger)) {
				return false;
			}
		}
		if (inst.opcode == Halt) {
			printf("VM halted!\n");
			fflush(stdout);
			handleInteractive(debugger);
			break;
		}
		if (NanoVMExecute(&debugger->vm, &inst)) {
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
	printf("VM exited with return code: %" PRId64 "\n", debugger->vm.cpu.registers[Reg0]);
	fflush(stdout);
	handleInteractive(debugger);
	return true;
}
