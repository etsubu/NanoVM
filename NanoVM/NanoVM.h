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
#define OPCODE_MASK	0b00011111
#define DST_REG_MASK	0b11100000
#define SRC_TYPE_MASK	0b10000000
#define SRC_SIZE_MASK	0b01100000
#define DST_MEM_MASK	0b00010000
#define SRC_MEM_MASK	0b00001000
#define SRC_REG_MASK	0b00000111

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
	esp,
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
	Sar,
	Sal,
	Ror,
	Rol,
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
	Inc,
	Dec,
	Ret,

	Call,
	Push,
	Pop,
	Halt,
	Printi,
	Prints,
	Printc,
	Syscall,
	Memcpy
};

#ifndef TYPE_H
#define TYPE_H

enum Size {
	Byte,
	Short,
	Dword,
	Qword
};

enum DataType {
	Reg,
	Immediate
};

#endif

struct NanoVMCpu {
	uint64_t registers[10];
	unsigned char* codeBase;
	unsigned char* stackBase;
	uint64_t codeSize;
	uint64_t stackSize;
	uint64_t bytecodeSize;
};

struct Instruction {
	unsigned char opcode;
	unsigned char dstReg;
	unsigned char srcReg;
	unsigned char srcType;
	bool isDstMem;
	bool isSrcMem;
	unsigned char srcSize;
	uint64_t immediate;
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
