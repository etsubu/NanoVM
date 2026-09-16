#include "NanoVM.h"
#include <inttypes.h>
#include <stdio.h>

static int64_t wrapAdd(int64_t a, int64_t b) {
    return (int64_t) ((uint64_t) a + (uint64_t) b);
}

int NanoVMPush(NanoVM *vm, int64_t value) {
    if ((uint64_t)vm->cpu.registers[sp] + sizeof(int64_t) > vm->cpu.codeSize + vm->cpu.stackSize) {
        vm->errorFlag |= STACK_ERROR;
        return 1;
    }
    if (vm->cpu.registers[sp] < (int64_t) (vm->cpu.stackBase - vm->cpu.memoryBase)) {
        vm->errorFlag |= STACK_ERROR;
        return 1;
    }
    memcpy(vm->cpu.memoryBase + vm->cpu.registers[sp], &value, sizeof(int64_t));
    vm->cpu.registers[sp] += sizeof(value);
    return 0;
}

int NanoVMPop(NanoVM *vm, int64_t *out) {
    if (vm->cpu.registers[sp] < (int64_t) (vm->cpu.codeSize + sizeof(int64_t)) || vm->cpu.registers[sp] > (int64_t) (vm->cpu.codeSize + vm->cpu.stackSize)) {
        vm->errorFlag |= STACK_ERROR;
        return 1;
    }
    void *ptr = (vm->cpu.memoryBase + vm->cpu.registers[sp] - sizeof(int64_t));
    memcpy(out, ptr, sizeof(int64_t));
    vm->cpu.registers[sp] -= sizeof(int64_t);
    return 0;
}

int NanoVMWriteMemory(NanoVM *vm, uint64_t vmAddress, void *buf, size_t size) {
    if (vmAddress >= vm->cpu.memorySize || size > vm->cpu.memorySize - vmAddress) {
        return 1;
    }
    memcpy(vm->cpu.memoryBase + vmAddress, buf, size);
    return 0;
}

int NanoVMReadMemory(NanoVM *vm, void *buf, uint64_t vmAddress, size_t size) {
    if (vmAddress >= vm->cpu.memorySize || size > vm->cpu.memorySize - vmAddress) {
        return 1;
    }
    memcpy(buf, vm->cpu.memoryBase + vmAddress, size);
    return 0;
}

int NanoVMInit(NanoVM *vm, unsigned char *code, uint64_t size) {
    vm->errorFlag = 0;
    vm->halted = 0;
    vm->syscall_table_size = 0;
    vm->syscall_table = 0;
    int heap_size = NANOVM_PAGE_SIZE;

    memset(&vm->cpu, 0, sizeof(vm->cpu));
    memset(vm->cpu.registers, 0, sizeof(vm->cpu.registers));

    if (size > NANOVM_MAX_CODE_SIZE) {
        return 1;
    }
    vm->cpu.codeSize = (NANOVM_PAGE_SIZE * (1 + (size / NANOVM_PAGE_SIZE)));
    vm->cpu.stackSize = NANOVM_PAGE_SIZE;
    size_t memorySize = vm->cpu.stackSize + vm->cpu.codeSize + heap_size;

    vm->cpu.memoryBase = (unsigned char *) malloc(memorySize + 10);
    if (!vm->cpu.memoryBase) {
        return 2;
    }
    memset(vm->cpu.memoryBase, 0, memorySize + 10);

    vm->cpu.stackBase = vm->cpu.memoryBase + vm->cpu.codeSize;
    memcpy(vm->cpu.memoryBase, code, size);

    vm->cpu.heapBase = vm->cpu.stackBase + vm->cpu.stackSize;

    vm->cpu.memorySize = memorySize;
    vm->cpu.registers[ip] = 0;
    vm->cpu.registers[sp] = vm->cpu.codeSize;
    vm->cpu.registers[bp] = vm->cpu.codeSize;

    NanoHeapInit(&vm->cpu.heap, vm->cpu.heapBase, heap_size);
    return 0;
}

int NanoVMInitFromFile(NanoVM *vm, const char *fileName) {
    vm->errorFlag = 0;
    vm->halted = 0;
    vm->syscall_table_size = 0;
    vm->syscall_table = 0;
    int heap_size = NANOVM_PAGE_SIZE;
    memset(&vm->cpu, 0, sizeof(vm->cpu));
    memset(vm->cpu.registers, 0, sizeof(vm->cpu.registers));

    long int size;

    FILE *file = fopen(fileName, "rb");
    if (file != NULL) {
        if (fseek(file, 0, SEEK_END)) {
            fclose(file);
            return 1;
        }
        size = ftell(file);
        if (size == -1) {
            fclose(file);
            return 1;
        }
        if ((uint64_t)size > NANOVM_MAX_CODE_SIZE) {
            return 1;
        }

        vm->cpu.codeSize = (NANOVM_PAGE_SIZE * (1 + (size / NANOVM_PAGE_SIZE)));
        vm->cpu.stackSize = NANOVM_PAGE_SIZE;

        size_t memorySize = vm->cpu.stackSize + vm->cpu.codeSize + heap_size;
        vm->cpu.memoryBase = (unsigned char *) malloc(memorySize + 10);
        if (!vm->cpu.memoryBase) {
            fclose(file);
            return 2;
        }
        memset(vm->cpu.memoryBase, 0, memorySize + 10);

        if (fseek(file, 0, SEEK_SET)) {
            fclose(file);
            free(vm->cpu.memoryBase);
            vm->cpu.memoryBase = 0;
            return 1;
        }
        size_t readBytes = fread(vm->cpu.memoryBase, 1, size, file);
        if (readBytes != (size_t) size) {
            fclose(file);
            free(vm->cpu.memoryBase);
            vm->cpu.memoryBase = 0;
            return 1;
        }
        (void) readBytes;
        fclose(file);

        vm->cpu.stackBase = vm->cpu.memoryBase + vm->cpu.codeSize;
        vm->cpu.memorySize = memorySize;
        vm->cpu.registers[sp] = vm->cpu.codeSize;
        vm->cpu.registers[bp] = vm->cpu.codeSize;
        vm->cpu.registers[ip] = 0;

        vm->cpu.heapBase = vm->cpu.stackBase + vm->cpu.stackSize;
        NanoHeapInit(&vm->cpu.heap, vm->cpu.heapBase, heap_size);
        return 0;
    }
    printf("Unable to open file");
    return 1;
}

void NanoVMDestroy(NanoVM *vm) {
    if (vm->cpu.memoryBase) {
        free(vm->cpu.memoryBase);
        vm->cpu.memoryBase = 0;
    }
}

int NanoVMAttachSyscallTable(NanoVM *vm, nanovm_syscall *syscall_table, size_t syscall_table_size) {
    vm->syscall_table = syscall_table;
    vm->syscall_table_size = syscall_table_size;
    return 0;
}

int64_t NanoVMRun(NanoVM *vm) {
    while (!vm->halted) {
        Instruction inst;
        if (!NanoVMFetch(vm, &inst)) {
            if (NanoVMExecute(vm, &inst)) {
                break;
            }
        } else {
            break;
        }
    }
    if (vm->errorFlag & MEMORY_ACCESS) {
        printf("Memory access error\n");
        return 1;
    }
    if (vm->errorFlag & OUT_OF_HEAP) {
        printf("Out of heap error\n");
        return 1;
    }
    if (vm->errorFlag & STACK_ERROR) {
        printf("Stack error\n");
        return 1;
    }
    if (vm->errorFlag & HEAP_ERROR) {
        printf("Heap error\n");
        return 1;
    }
    if (vm->errorFlag & SIGFPE_ERROR) {
        printf("Encountered division by 0\n");
        return 1;
    }
    if (vm->errorFlag & SHIFT_OUT_OF_RANGE) {
        printf("Shift operation out of range error [0, 64]\n");
        return 1;
    }
    if (vm->errorFlag & INVALID_OPCODE) {
        printf("Encountered invalid opcode\n");
        return 1;
    }
    if (vm->errorFlag & IP_ERROR) {
        printf("Instruction pointer outside of memory range\n");
        return 1;
    }
    if (vm->errorFlag & INVALID_SYSCALL) {
        printf("Encountered invalid syscall\n");
        return 1;
    }
    if (vm->errorFlag) {
        printf("Unknown error flag set\n");
        return 1;
    }
    return vm->cpu.registers[Reg0];
}

static int64_t allocate(NanoVM *vm, int64_t *dst, int64_t sourceValue) {
    if (sourceValue < 0) {
        vm->errorFlag |= OUT_OF_HEAP;
        return 1;
    }
    uint64_t delta;
    if (NanoHeapAllocate(&vm->cpu.heap, (uint64_t) sourceValue, &delta)) {
        vm->errorFlag |= OUT_OF_HEAP;
        return 1;
    }
    *dst = (int64_t) (delta + (vm->cpu.heapBase - vm->cpu.memoryBase));
    return 0;
}

static int64_t heap_free(NanoVM *vm, int64_t sourceValue) {
    uint64_t heapBaseAddr = (uint64_t) (vm->cpu.heapBase - vm->cpu.memoryBase);
    uint64_t addr = (uint64_t) sourceValue;
    if (addr < heapBaseAddr || addr - heapBaseAddr >= vm->cpu.heap.heap_size) {
        vm->errorFlag |= HEAP_ERROR;
        return 1;
    }
    if (NanoHeapFree(&vm->cpu.heap, addr - heapBaseAddr)) {
        vm->errorFlag |= HEAP_ERROR;
        return 1;
    }
    return 0;
}

int NanoVMExecute(NanoVM *vm, Instruction *inst) {
    int64_t *dst;
    int64_t sourceValue;
    dst = &vm->cpu.registers[inst->dstReg];
    if (inst->srcType == Reg) {
        sourceValue = vm->cpu.registers[inst->srcReg];
    } else {
        sourceValue = inst->immediate;
    }
    switch (inst->opcode) {
        case Mov:
            *dst = sourceValue;
            break;
        case Add:
            *dst = (int64_t) ((uint64_t) *dst + (uint64_t) sourceValue);
            break;
        case Sub:
            *dst = (int64_t) ((uint64_t) *dst - (uint64_t) sourceValue);
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
            if (sourceValue < 0 || sourceValue >= 64) {
                vm->errorFlag |= SHIFT_OUT_OF_RANGE;
                return 1;
            }
            *dst >>= sourceValue;
            break;
        case Shl:
            if (sourceValue < 0 || sourceValue >= 64) {
                vm->errorFlag |= SHIFT_OUT_OF_RANGE;
                return 1;
            }
            *dst = (int64_t) ((uint64_t) *dst << sourceValue);
            break;
        case Mul:
            *dst = (int64_t) ((uint64_t) *dst * (uint64_t) sourceValue);
            break;
        case Div:
            if (sourceValue == 0) {
                vm->errorFlag |= SIGFPE_ERROR;
                return 1;
            }
            if (*dst == INT64_MIN && sourceValue == -1) {
                *dst = INT64_MIN;
                break;
            }
            *dst /= sourceValue;
            break;
        case Mod:
            if (sourceValue == 0) {
                vm->errorFlag |= SIGFPE_ERROR;
                return 1;
            }
            if (*dst == INT64_MIN && sourceValue == -1) {
                *dst = 0;
                break;
            }
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
                vm->cpu.registers[ip] = wrapAdd(vm->cpu.registers[ip], sourceValue);
                return 0;
            }
            break;
        case Jnz:
            if (!(vm->cpu.registers[flags] & ZERO_FLAG)) {
                vm->cpu.registers[ip] = wrapAdd(vm->cpu.registers[ip], sourceValue);
                return 0;
            }
            break;
        case Jg:
            if (vm->cpu.registers[flags] & GREATER_FLAG) {
                vm->cpu.registers[ip] = wrapAdd(vm->cpu.registers[ip], sourceValue);
                return 0;
            }
            break;
        case Js:
            if (vm->cpu.registers[flags] & SMALLER_FLAG) {
                vm->cpu.registers[ip] = wrapAdd(vm->cpu.registers[ip], sourceValue);
                return 0;
            }
            break;
        case Jmp:
            vm->cpu.registers[ip] = wrapAdd(vm->cpu.registers[ip], sourceValue);
            return 0;
        case Not:
            vm->cpu.registers[inst->srcReg] = ~vm->cpu.registers[inst->srcReg];
            break;
        case Call:
            if (NanoVMPush(vm, vm->cpu.registers[ip] + inst->instructionSize)) {
                // stack overflow
                return 1;
            }
            vm->cpu.registers[ip] = wrapAdd(vm->cpu.registers[ip], sourceValue);
            return 0;
        case Push:
            if (NanoVMPush(vm, sourceValue)) {
                // Stack overflow
                return 1;
            }
            break;
        case Pop:
            if (NanoVMPop(vm, &vm->cpu.registers[inst->srcReg])) {
                // Stack underflow
                return 1;
            }
            break;
        case Ret:
            if (NanoVMPop(vm, &vm->cpu.registers[ip])) {
                // stack underflow
                return 1;
            }
            return 0;
        case Load: {
            int64_t value;
            if (NanoVMReadMemory(vm, &value, sourceValue, sizeof(value))) {
                vm->errorFlag |= MEMORY_ACCESS;
                return 1;
            }
            *dst = value;
            break;
        }
        case Load8: {
            int8_t value;
            if (NanoVMReadMemory(vm, &value, sourceValue, sizeof(value))) {
                vm->errorFlag |= MEMORY_ACCESS;
                return 1;
            }
            *dst = value;
            break;
        }
        case Load8u: {
            uint8_t value;
            if (NanoVMReadMemory(vm, &value, sourceValue, sizeof(value))) {
                vm->errorFlag |= MEMORY_ACCESS;
                return 1;
            }
            *dst = value;
            break;
        }
        case Store:
            if (NanoVMWriteMemory(vm, *dst, &sourceValue, sizeof(int64_t))) {
                vm->errorFlag |= MEMORY_ACCESS;
                return 1;
            }
            break;
        case Store8:
            if (NanoVMWriteMemory(vm, *dst, &sourceValue, sizeof(int8_t))) {
                vm->errorFlag |= MEMORY_ACCESS;
                return 1;
            }
            break;
        case Syscall:
            if (vm->syscall_table_size == 0 || sourceValue < 0 || (uint64_t)sourceValue >= vm->syscall_table_size || !vm->syscall_table[sourceValue]) {
                vm->errorFlag |= INVALID_SYSCALL;
                return 1;
            }
            *dst = vm->syscall_table[sourceValue](&vm->cpu);
            break;
        case Printi:
            printf("%" PRId64 "\n", sourceValue);
            break;
        case Alloc:
            if (allocate(vm, dst, sourceValue)) {
                return 1;
            }
            break;
        case Free:
            if (heap_free(vm, sourceValue)) {
                return 1;
            }
            break;
        case Inc:
            vm->cpu.registers[inst->srcReg] = wrapAdd(vm->cpu.registers[inst->srcReg], 1);
            if (vm->cpu.registers[inst->srcReg] == 0) {
                vm->cpu.registers[flags] = ZERO_FLAG;
            } else {
                vm->cpu.registers[flags] = 0;
            }
            break;
        case Dec:
            vm->cpu.registers[inst->srcReg] = wrapAdd(vm->cpu.registers[inst->srcReg], -1);
            if (vm->cpu.registers[inst->srcReg] == 0) {
                vm->cpu.registers[flags] = ZERO_FLAG;
            } else {
                vm->cpu.registers[flags] = 0;
            }
            break;
        case Halt:
            vm->halted = 1;
            return 0;
        default:
            printf("Unknown opcode %d\n", inst->opcode);
            vm->errorFlag |= INVALID_OPCODE;
            return 1;
    }
    vm->cpu.registers[ip] += inst->instructionSize;
    return 0;
}

int NanoVMFetch(NanoVM *vm, Instruction *inst) {
    if ((uint64_t) vm->cpu.registers[ip] + 2 > vm->cpu.codeSize || vm->cpu.registers[ip] < 0) {
        vm->errorFlag |= IP_ERROR;
        return 1;
    }
    unsigned char *rawIp = vm->cpu.memoryBase + vm->cpu.registers[ip];
    uint16_t header;
    memcpy(&header, rawIp, sizeof(header));
    inst->opcode = (header & OPCODE_MASK) >> 7;
    inst->dstReg = ((header & DST_REG_MASK) >> 13);
    inst->srcType = (header & SRC_TYPE_MASK) >> 5;
    inst->srcReg = (header & SRC_REG_MASK);
    inst->srcSize = (header & SRC_SIZE_MASK) >> 3;

    if (inst->srcType == Immediate) {
        static const unsigned immBytes[4] = {1, 2, 4, 8};
        unsigned n = immBytes[inst->srcSize];
        if ((uint64_t) vm->cpu.registers[ip] + sizeof(header) + n > vm->cpu.codeSize) {
            vm->errorFlag |= IP_ERROR;
            return 1;
        }
        switch (inst->srcSize) {
            case bits8: {
                int8_t imm;
                memcpy(&imm, rawIp + sizeof(header), sizeof(imm));
                inst->immediate = imm;
                inst->instructionSize = 3;
                inst->srcSizeInBytes = 1;
                break;
            }
            case bits16: {
                int16_t imm;
                memcpy(&imm, rawIp + sizeof(header), sizeof(imm));
                inst->immediate = imm;
                inst->instructionSize = 4;
                inst->srcSizeInBytes = 2;
                break;
            }
            case bits32: {
                int32_t imm;
                memcpy(&imm, rawIp + sizeof(header), sizeof(imm));
                inst->immediate = imm;
                inst->instructionSize = 6;
                inst->srcSizeInBytes = 4;
                break;
            }
            case bits64:
                memcpy(&inst->immediate, rawIp + sizeof(header), sizeof(inst->immediate));
                inst->instructionSize = 10;
                inst->srcSizeInBytes = 8;
                break;
        }
    } else {
        inst->instructionSize = 2;
        inst->srcSizeInBytes = 8;
    }
    return 0;
}
