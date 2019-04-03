#pragma once

#include <iostream>
#include <fstream>

#define PAGE_SIZE 4096
#define OPCODE_MASK  0b00011111
#define DST_REG_MASK 0b11100000
#define SRC_TYPE	 0b10000000
#define SRC_SIZE	 0b01100000
#define DST_MEM		 0b00010000
#define SRC_MEM		 0b00001000
#define SRC_REG		 0b00000111

#define STACK_ERROR			0b10000000
#define IP_ERROR			0b01000000
#define MEMORY_ACCESS		0b00100000

#define ZERO_FLAG		0b10000000
#define GREATER_FLAG	0b01000000
#define SMALLER_FLAG	0b00100000

enum Size {
	Byte,
	Short,
	Dword,
	Qword
};

enum Register {
	Reg0,
	Reg1,
	Reg2,
	Reg3,
	Reg4,
	Reg5,
	Reg6,
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

enum Type {
	Reg,
	Immediate
};
struct NanoVMCpu{
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

class NanoVM {
public:
	NanoVM(unsigned char* code, uint64_t size);
	NanoVM(std::string file);
	~NanoVM();
	uint64_t Run();
protected:
	template<class T> T pop();
	template<class T> void push(T value);
	bool fetch(Instruction &instruction) const;
	bool execute(Instruction &instruction);
	unsigned char errorFlag;
	NanoVMCpu cpu;
};