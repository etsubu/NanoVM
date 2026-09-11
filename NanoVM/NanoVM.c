#include "NanoVM.h"
#include <inttypes.h>
#include <stdio.h>

int NanoVMPush(NanoVM *vm, int64_t value) {
	if (vm->cpu.registers[sp] + sizeof(int64_t) >= vm->cpu.memoryBase + vm->cpu.memorySize) {
		vm->errorFlag = STACK_ERROR;
		return 1;
	}
	if (vm->cpu.registers[sp] < vm->cpu.stackBase) {
		vm->errorFlag = STACK_ERROR;
		return 1;
	}
	*(int64_t*)(vm->cpu.stackBase + vm->cpu.registers[sp]) = value;
	vm->cpu.registers[sp] += sizeof(value);
	return 0;
}

int64_t NanoVMPop(NanoVM *vm) {
	if ((vm->cpu.registers[sp] - (uint64_t)vm->cpu.stackBase) < sizeof(int64_t)) {
		return vm->errorFlag = STACK_ERROR;
		return 0;
	}
	int64_t* value = (int64_t*)(vm->cpu.memoryBase + vm->cpu.registers[sp]);
	if (value >= vm->cpu.memoryBase + vm -> cpu.memorySize - sizeof(int64_t)) {
		return vm->errorFlag = STACK_ERROR;
		return 0;
	}
	vm->cpu.registers[sp] -= sizeof(int64_t);
	return *value;
}

void NanoVMInit(NanoVM* vm, unsigned char* code, uint64_t size) {
	memset(&vm->cpu, 0, sizeof(vm->cpu));
	memset(vm->cpu.registers, 0, sizeof(vm->cpu.registers));
	vm->cpu.codeSize = (NANOVM_PAGE_SIZE * (1 + (size / NANOVM_PAGE_SIZE)));
	vm->cpu.stackSize = NANOVM_PAGE_SIZE;
	size_t memorySize = vm->cpu.stackSize + vm->cpu.codeSize;
	vm->cpu.memoryBase = (unsigned char*)malloc(memorySize + 10);
	memset(vm->cpu.memoryBase, 0, memorySize + 10);
	vm->cpu.stackBase = vm->cpu.memoryBase + vm->cpu.codeSize;
	memcpy(vm->cpu.memoryBase, code, size);
	vm->cpu.memorySize = memorySize;
	vm->cpu.registers[ip] = 0;
	vm->cpu.registers[sp] = vm->cpu.codeSize;
	vm->cpu.registers[bp] = vm->cpu.codeSize;
}

void NanoVMInitFromFile(NanoVM* vm, const char* fileName) {
	memset(&vm->cpu, 0, sizeof(vm->cpu));
	memset(vm->cpu.registers, 0, sizeof(vm->cpu.registers));

	long size;

	FILE* file = fopen(fileName, "rb");
	if (file != NULL)
	{
		fseek(file, 0, SEEK_END);
		size = ftell(file);

		vm->cpu.codeSize = (NANOVM_PAGE_SIZE * (1 + (size / NANOVM_PAGE_SIZE)));
		vm->cpu.stackSize = NANOVM_PAGE_SIZE;

		size_t memorySize = vm->cpu.stackSize + vm->cpu.codeSize;
		vm->cpu.memoryBase = (unsigned char*)malloc(memorySize + 10);
		memset(vm->cpu.memoryBase, 0, memorySize + 10);

		fseek(file, 0, SEEK_SET);
		size_t readBytes = fread(vm->cpu.memoryBase, 1, size, file);
		(void)readBytes;
		fclose(file);

		vm->cpu.stackBase = vm->cpu.memoryBase + vm->cpu.codeSize;
		vm->cpu.memorySize = memorySize;
		vm->cpu.registers[sp] = vm->cpu.codeSize;
		vm->cpu.registers[bp] = vm->cpu.codeSize;
		vm->cpu.registers[ip] = 0;
	}
	else printf("Unable to open file");
}

void NanoVMDestroy(NanoVM* vm) {
	free(vm->cpu.memoryBase);
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
	int64_t *dst, *src;
	int64_t sourceValue;
	dst = &vm->cpu.registers[inst->dstReg];
	if (inst->srcType == Reg) {
		sourceValue = vm->cpu.registers[inst->srcReg];
	}
	else {
		if (inst->srcSize == bits8) {
			sourceValue = inst->immediate & 0xFF;
		} else if (inst->srcSize == bits16) {
			sourceValue = inst->immediate & 0xFFFF;
		}else if (inst->srcSize == bits16) {
			sourceValue = inst->immediate & 0xFFFF;
		} else {
			sourceValue = inst->immediate;
		}
	}
	switch (inst->opcode) {
		case Mov:
			*dst = sourceValue;
			break;
		case Add:
			*dst += sourceValue;
			break;
		case Sub:
			*dst -= sourceValue;
			break;
		case And:
			*dst &= sourceValue;
			break;
		case Or:
			*dst |= sourceValue;
			break;
		case Xor:
			*dst ^= sourceValue;
			break;
		case Shr:
			*dst >>= sourceValue;
			break;
		case Shl:
			*dst <<= sourceValue;
			break;
		case Mul:
			*dst *= sourceValue;
			break;
		case Div:
			*dst /= sourceValue;
			break;
		case Mod:
			*dst %= sourceValue;
			break;
		case Cmp:
			vm->cpu.registers[flags] = 0;
			if (*dst == sourceValue) {
				vm->cpu.registers[flags] |= ZERO_FLAG;
			} else if (*dst > sourceValue) {
				vm->cpu.registers[flags] |= GREATER_FLAG;
			} else {
				vm->cpu.registers[flags] |= SMALLER_FLAG;
			}
			break;
		case Jz:
			if (vm->cpu.registers[flags] & ZERO_FLAG) {
				vm->cpu.registers[ip] += sourceValue;
				return true;
			}
			break;
		case Jnz:
			if (!(vm->cpu.registers[flags] & ZERO_FLAG)) {
				vm->cpu.registers[ip] += sourceValue;
				return true;
			}
			break;
		case Jg:
			if (!(vm->cpu.registers[flags] & GREATER_FLAG)) {
				vm->cpu.registers[ip] += sourceValue;
				return true;
			}
			break;
		case Js:
			if (!(vm->cpu.registers[flags] & SMALLER_FLAG)) {
				vm->cpu.registers[ip] += sourceValue;
				return true;
			}
			break;
		case Jmp:
			vm->cpu.registers[ip] += sourceValue;
			return true;
		case Not:
			*dst = ~(*dst);
			break;
		case Call:
			NanoVMPush(vm, vm->cpu.registers[ip] + inst->instructionSize);
			vm->cpu.registers[ip] += sourceValue;
			break;
		case Push:
			NanoVMPush(vm, sourceValue);
			break;
		case Pop:
			*dst = NanoVMPop(vm);
			break;
		case Ret:
			vm->cpu.registers[ip] = NanoVMPop(vm);
			break;
		case Load:
			break;
		case Store:
			break;
		case Syscall:
			break;
		case Printi:
			printf("%d\n", sourceValue);
	}
	vm->cpu.registers[ip] += inst->instructionSize;
	return true;
}

bool NanoVMFetch(const NanoVM* vm, Instruction* inst) {
	if (vm->cpu.registers[ip] >= vm->cpu.codeSize || vm->cpu.registers[ip] < 0) {
		printf("IP out of bounds\n");
		return false;
	}
	unsigned char* rawIp = vm->cpu.memoryBase + vm->cpu.registers[ip];
	uint64_t value = *(uint64_t*)rawIp;
	inst->opcode   =  (value & OPCODE_MASK) >> 7;
	inst->dstReg   =  ((value & DST_REG_MASK) >> 13);
	inst->srcType  =  (value  & SRC_TYPE_MASK) >> 5;
	inst->srcReg   =   (value & SRC_REG_MASK);
	inst->srcSize  =  (value & SRC_SIZE_MASK) >> 3;
	if (inst->srcType == Immediate) {
		switch (inst->srcSize) {
		case bits8:
			inst->immediate = (int8_t)(value >> 16);
			inst->instructionSize = 3;
			break;
		case bits16:
			inst->immediate = (int16_t)(value >> 16);
			inst->instructionSize = 4;
			break;
		case bits32:
			inst->immediate = (int32_t)(value >> 16);
			inst->instructionSize = 6;
			break;
		case bits64:
			inst->immediate = *(int64_t*)(rawIp + 2);
			inst->instructionSize = 10;
			break;
		}
	}
	else {
		inst->instructionSize = 2;
	}
	return true;

}
