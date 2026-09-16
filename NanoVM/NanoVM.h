#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "NanoHeap.h"

#ifdef __cplusplus
extern "C" {

#endif

#define NANOVM_PAGE_SIZE	4096
// Largest accepted program. Bounding the input keeps the page rounding in
// NanoVMInit from wrapping and caps how much memory a caller can ask for.
#define NANOVM_MAX_CODE_SIZE	(64ull * 1024 * 1024)
#define DST_REG_MASK	0xE000	// 0b1110000000000000
#define OPCODE_MASK		0x1F80	// 0b0001111110000000
#define RESERVED_MASK	0x0040	// 0b0000000001000000
#define SRC_TYPE_MASK	0x0020	// 0b0000000000100000
#define SRC_SIZE_MASK	0x0018	// 0b0000000000011000
#define SRC_REG_MASK	0x0007	// 0b0000000000000111

#define STACK_ERROR		0x80	// 0b10000000
#define IP_ERROR		0x40	// 0b01000000
#define MEMORY_ACCESS	0x20	// 0b00100000
#define OUT_OF_HEAP		0x10	// 0b00010000
#define HEAP_ERROR		0x08	// 0b00001000
#define SIGFPE_ERROR	0x04	// 0b00000100
#define SHIFT_OUT_OF_RANGE	0x02	// 0b00000010
#define INVALID_OPCODE	0x01	// 0b00000001
#define INVALID_SYSCALL	0x0100	// 0b0000000100000000

#define ZERO_FLAG	0x80	// 0b10000000
#define GREATER_FLAG	0x40	// 0b01000000
#define SMALLER_FLAG	0x20	// 0b00100000

enum Register {
    Reg0,
    Reg1,
    Reg2,
    Reg3,
    Reg4,
    Reg5,
    bp,
    sp,
    ip,
    flags
};

enum Opcodes {
    Mov,
    Add,
    Sub,
    And,
    Or,
    Xor,
    Shr,
    Shl,
    Mul,
    Div,
    Mod,
    Cmp,

    Jz,
    Jnz,
    Jg,
    Js,
    Jmp,
    Not,

    Call,
    Push,
    Pop,
    Ret,
    Halt,
    Load,
    Load8,
    Load8u,
    Store,
    Store8,
    Syscall,

    Printi,
    Alloc,
    Free,

    Inc,
    Dec
};

#ifndef TYPE_H
#define TYPE_H

enum Size {
    bits8,
    bits16,
    bits32,
    bits64
};

enum DataType {
    Reg,
    Immediate
};

#endif

struct NanoVMCpu {
    int64_t registers[10];
    unsigned char *memoryBase;
    unsigned char *stackBase;
    unsigned char *heapBase;
    NanoHeap heap;
    uint64_t codeSize;
    uint64_t stackSize;
    size_t memorySize;
};

struct Instruction {
    unsigned char opcode;
    unsigned char dstReg;
    unsigned char srcReg;
    unsigned char srcType;
    unsigned char srcSize;
    int64_t immediate;
    unsigned srcSizeInBytes;
    unsigned char instructionSize;
};

typedef struct NanoVMCpu NanoVMCpu;
typedef struct Instruction Instruction;

// Syscall function signature
typedef int64_t (*nanovm_syscall)(NanoVMCpu *cpu);

struct NanoVM {
    uint16_t errorFlag;
    unsigned char halted;
    NanoVMCpu cpu;
    nanovm_syscall *syscall_table;
    size_t syscall_table_size;
};

typedef struct NanoVM NanoVM;

int NanoVMInit(NanoVM *vm, unsigned char *code, uint64_t size);

int NanoVMAttachSyscallTable(NanoVM *vm, nanovm_syscall *syscall_table, size_t syscall_table_size);

int NanoVMInitFromFile(NanoVM *vm, const char *fileName);

int NanoVMPush(NanoVM *vm, int64_t value);

int NanoVMPop(NanoVM *vm, int64_t *out);

int NanoVMWriteMemory(NanoVM *vm, uint64_t vmAddress, void *buf, size_t size);

int NanoVMReadMemory(NanoVM *vm, void *buf, uint64_t vmAddress, size_t size);

void NanoVMDestroy(NanoVM *vm);

int64_t NanoVMRun(NanoVM *vm);

int NanoVMFetch(NanoVM *vm, Instruction *instruction);

int NanoVMExecute(NanoVM *vm, Instruction *instruction);

#ifdef __cplusplus
}
#endif
