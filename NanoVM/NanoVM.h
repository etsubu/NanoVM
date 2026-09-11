#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NANOVM_PAGE_SIZE	4096
#define DST_REG_MASK	0b1110000000000000
#define OPCODE_MASK		0b0001111110000000
#define RESERVED_MASK	0b0000000001000000
#define SRC_TYPE_MASK	0b0000000000100000
#define SRC_SIZE_MASK	0b0000000000011000
#define SRC_REG_MASK	0b0000000000000111

#define STACK_ERROR	0b10000000
#define IP_ERROR	0b01000000
#define MEMORY_ACCESS	0b00100000

#define ZERO_FLAG	0b10000000
#define GREATER_FLAG	0b01000000
#define SMALLER_FLAG	0b00100000

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
	Store,
	Syscall,

	Printi
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
	unsigned char* memoryBase;
	unsigned char* stackBase;
	unsigned char* heapBase;
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
	unsigned char instructionSize;
};

typedef struct NanoVMCpu NanoVMCpu;
typedef struct Instruction Instruction;

struct NanoVM {
	unsigned char errorFlag;
	NanoVMCpu cpu;
};

typedef struct NanoVM NanoVM;

void NanoVMInit(NanoVM* vm, unsigned char* code, uint64_t size);

void NanoVMInitFromFile(NanoVM* vm, const char* fileName);

void NanoVMDestroy(NanoVM* vm);

uint64_t NanoVMRun(NanoVM* vm);

bool NanoVMFetch(const NanoVM* vm, Instruction* instruction);

bool NanoVMExecute(NanoVM* vm, Instruction* instruction);

#ifdef __cplusplus
}
#endif
