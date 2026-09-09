#include "NanoVM.h"
#include <inttypes.h>

#define PUSH(TYPE, VALUE) \
	do { \
		TYPE pushValue = (VALUE); \
		if (sizeof(TYPE) + vm->cpu.registers[esp] >= (uint64_t)vm->cpu.stackBase + vm->cpu.stackSize) { \
			vm->errorFlag = STACK_ERROR; \
			break; \
		} \
		*(TYPE*)(vm->cpu.codeBase + vm->cpu.registers[esp]) = pushValue; \
		vm->cpu.registers[esp] += sizeof(TYPE); \
	} while (0)

#define POP(TYPE, DST) \
	do { \
		if (vm->cpu.registers[esp] - sizeof(TYPE) < vm->cpu.codeSize) { \
			vm->errorFlag = STACK_ERROR; \
			(DST) = 0; \
			break; \
		} \
		TYPE popValue = *(TYPE*)(vm->cpu.codeBase + vm->cpu.registers[esp] - sizeof(TYPE)); \
		vm->cpu.registers[esp] -= sizeof(TYPE); \
		(DST) = popValue; \
	} while (0)

void NanoVMInit(NanoVM* vm, unsigned char* code, uint64_t size) {
	vm->cpu.bytecodeSize = size;
	memset(&vm->cpu, 0x00, sizeof(vm->cpu));
	memset(vm->cpu.registers, 0x00, sizeof(vm->cpu.registers));
	vm->cpu.codeSize = (NANOVM_PAGE_SIZE * (1 + (size / NANOVM_PAGE_SIZE)));
	vm->cpu.stackSize = NANOVM_PAGE_SIZE;
	vm->cpu.codeBase = (unsigned char*)malloc(vm->cpu.stackSize + 10 + vm->cpu.codeSize);
	vm->cpu.stackBase = vm->cpu.codeBase + vm->cpu.codeSize;
	memset(vm->cpu.stackBase, 0x00, sizeof(vm->cpu.stackSize) + 10);
	memcpy(vm->cpu.codeBase, code, size);
	vm->cpu.registers[ip] = 0;
	vm->cpu.registers[esp] = vm->cpu.codeSize;
	vm->cpu.registers[bp] = vm->cpu.codeSize;
}

void NanoVMInitFromFile(NanoVM* vm, const char* fileName) {
	memset(&vm->cpu, 0x00, sizeof(vm->cpu));
	memset(vm->cpu.registers, 0x00, sizeof(vm->cpu.registers));

	long size;

	FILE* file = fopen(fileName, "rb");
	if (file != NULL)
	{
		fseek(file, 0, SEEK_END);
		size = ftell(file);

		vm->cpu.codeSize = (NANOVM_PAGE_SIZE * (1 + (size / NANOVM_PAGE_SIZE)));
		vm->cpu.stackSize = NANOVM_PAGE_SIZE;

		vm->cpu.codeBase = (unsigned char*)malloc(vm->cpu.stackSize + 10 + vm->cpu.codeSize);

		fseek(file, 0, SEEK_SET);
		size_t readBytes = fread(vm->cpu.codeBase, 1, size, file);
		(void)readBytes;
		fclose(file);

		vm->cpu.bytecodeSize = size;
		vm->cpu.stackBase = vm->cpu.codeBase + vm->cpu.codeSize;
		memset(vm->cpu.stackBase, 0x00, sizeof(vm->cpu.stackSize) + 10);
		vm->cpu.registers[esp] = vm->cpu.codeSize;
		vm->cpu.registers[bp] = vm->cpu.codeSize;
		vm->cpu.registers[ip] = 0;
	}
	else printf("Unable to open file");
}

void NanoVMDestroy(NanoVM* vm) {
	free(vm->cpu.codeBase);
}

uint64_t NanoVMRun(NanoVM* vm) {
	while (true) {
		Instruction inst;
		if (NanoVMFetch(vm, &inst)) {
			if (inst.opcode == Halt) {
				return vm->cpu.registers[Reg0];
			}
			if (!NanoVMExecute(vm, &inst)) {
				switch (vm->errorFlag) {
				case MEMORY_ACCESS:
					return 1;
					break;
				default:
					return 2;
				}
				return false;
			}
		}
		else {
			return 3;
		}
	}
}

bool NanoVMExecute(NanoVM* vm, Instruction* inst) {
	void *dst, *src;
	bool isDstReg = false;
	dst = (inst->isDstMem) ? (void*)(vm->cpu.codeBase + vm->cpu.registers[inst->dstReg]) : (void*)&vm->cpu.registers[inst->dstReg];
	if (inst->srcType == Reg) {
		src = (inst->isSrcMem) ? (void*)(vm->cpu.codeBase + vm->cpu.registers[inst->srcReg]) : (void*)&vm->cpu.registers[inst->srcReg];
	}
	else {
		isDstReg = (inst->isDstMem) ? false : true;
		src = (inst->isSrcMem) ? (void*)(vm->cpu.codeBase + inst->immediate) : (void*)&inst->immediate;
	}
	if ((src != &inst->immediate && src != &vm->cpu.registers[inst->srcReg] && ((unsigned char*)src < vm->cpu.codeBase || (unsigned char*)src >= vm->cpu.codeBase + vm->cpu.codeSize + vm->cpu.stackSize)) || (dst != &vm->cpu.registers[inst->dstReg] && ((unsigned char*)dst < vm->cpu.codeBase || (unsigned char*)dst > vm->cpu.codeBase + vm->cpu.codeSize + vm->cpu.stackSize))) {
		vm->errorFlag = MEMORY_ACCESS;
		return false;
	}

	#define MATHOP(INST, OP, SIZE, DSTSIZE) \
    case INST: {         \
		*(DSTSIZE*)dst OP *(SIZE*)src; \
		break; \
    }

	#define BRANCH(USIZE, SIZE, DSTSIZE) \
	switch(inst->opcode) { \
		MATHOP(Add, +=, USIZE, DSTSIZE) \
		MATHOP(Mov, =, USIZE, DSTSIZE) \
		MATHOP(Sub, -=, USIZE, DSTSIZE) \
		MATHOP(Xor, ^=, USIZE, DSTSIZE) \
		MATHOP(And, &=, USIZE, DSTSIZE) \
		MATHOP(Or, |=, USIZE, DSTSIZE) \
		MATHOP(Sar, >>=, USIZE, DSTSIZE) \
		MATHOP(Sal, <<=, USIZE, DSTSIZE) \
		MATHOP(Div, /=, USIZE, DSTSIZE) \
		MATHOP(Mul, *=, USIZE, DSTSIZE) \
		MATHOP(Mod, %=, USIZE, DSTSIZE) \
	case Printi: \
		printf("%" PRIu64 "", *(USIZE*)src); \
		break; \
	case Prints: \
		printf("%s", (char*)src); \
		break; \
	case Printc: \
		printf("%c", *(USIZE*)src); \
		break; \
	case Inc: \
		*(USIZE*)src += 1; \
		break; \
	case Dec: \
		*(USIZE*)src -= 1; \
		break; \
	case Push: \
		PUSH(USIZE, *(USIZE*)src); \
		break; \
	case Pop: \
		POP(USIZE, *(USIZE*)dst); \
		break; \
	case Jz: \
		if (vm->cpu.registers[flags] & ZERO_FLAG) { \
			vm->cpu.registers[ip] += *(SIZE*)src; \
			return true; \
		} \
		break; \
	case Jnz: \
		if (!(vm->cpu.registers[flags] & ZERO_FLAG)) { \
			vm->cpu.registers[ip] += *(SIZE*)src; \
			return true; \
		} \
		break; \
	case Jg: \
		if (vm->cpu.registers[flags] & GREATER_FLAG) { \
			vm->cpu.registers[ip] += *(SIZE*)src; \
			return true; \
		} \
		break; \
	case Js: \
		if (vm->cpu.registers[flags] & SMALLER_FLAG) { \
			vm->cpu.registers[ip] += *(SIZE*)src; \
			return true; \
		} \
		break; \
	case Jmp: \
		vm->cpu.registers[ip] += *(SIZE*)src; \
		return true; \
	case Call: \
		PUSH(uint64_t, vm->cpu.registers[ip] + inst->instructionSize); \
		vm->cpu.registers[ip] += *(SIZE*)src; \
		return true; \
	case Ret: \
		POP(uint64_t, vm->cpu.registers[ip]); \
		return true; \
	case Cmp: \
		if (*(USIZE*)dst == *(USIZE*)src) \
			vm->cpu.registers[flags] = ZERO_FLAG; \
		else if (*(USIZE*)dst > *(USIZE*)src) \
			vm->cpu.registers[flags] = GREATER_FLAG; \
		else \
			vm->cpu.registers[flags] = SMALLER_FLAG; \
		break; \
	default: \
		return false; \
	}


	switch (inst->srcSize) {
	case Byte:
		if (isDstReg) {
			BRANCH(uint8_t, int8_t, uint64_t);
		}
		else {
			BRANCH(uint8_t, int8_t, uint8_t);
		}
		break;
	case Short:
		if (isDstReg) {
			BRANCH(uint16_t, int16_t, uint64_t);
		}
		else {
			BRANCH(uint16_t, int16_t, uint16_t);
		}
		break;
	case Dword:
		if (isDstReg) {
			BRANCH(uint32_t, int32_t, uint64_t);
		}
		else {
			BRANCH(uint32_t, int32_t, uint32_t);
		}
		break;
	default:
		BRANCH(uint64_t, int64_t, uint64_t);
		break;
	}
	vm->cpu.registers[ip] += inst->instructionSize;
	return true;
}

bool NanoVMFetch(const NanoVM* vm, Instruction* inst) {
	if (vm->cpu.registers[ip] >= vm->cpu.codeSize) {
		printf("IP out of bounds\n");
		return false;
	}
	unsigned char* rawIp = vm->cpu.codeBase + vm->cpu.registers[ip];
	uint64_t value = *(uint64_t*)rawIp;
	inst->opcode   =  (value & (unsigned char)OPCODE_MASK);
	inst->dstReg   =  ((value & DST_REG_MASK) >> 5);
	inst->srcType  =  (value >> 8) & SRC_TYPE_MASK;
	inst->srcReg   =   (value >> 8) & SRC_REG_MASK;
	inst->srcSize  =  ((value >> 8) & SRC_SIZE_MASK) >> 5;
	inst->isDstMem =  ((value >> 8) & DST_MEM_MASK);
	inst->isSrcMem =  ((value >> 8) & SRC_MEM_MASK);
	if (inst->srcType) {
		switch (inst->srcSize) {
		case Byte:
			inst->immediate = (uint8_t)(value >> 16);
			inst->instructionSize = 3;
			break;
		case Short:
			inst->immediate = (uint16_t)(value >> 16);
			inst->instructionSize = 4;
			break;
		case Dword:
			inst->immediate = (uint32_t)(value >> 16);
			inst->instructionSize = 6;
			break;
		case Qword:
			inst->immediate = *(uint64_t*)(rawIp + 2);
			inst->instructionSize = 10;
			break;
		}
	}
	else {
		inst->instructionSize = 2;
	}
	return true;

}
