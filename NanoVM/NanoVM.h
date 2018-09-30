#pragma once

#include <iostream>
#include <fstream>

#define PAGE_SIZE 4096
#define OPCODE_MASK  0b11111000
#define DST_REG_MASK 0b00000111
#define SRC_TYPE	 0b10000000
#define SRC_SIZE	 0b01100000
#define DST_MEM		 0b00010000
#define SRC_MEM		 0b00001000
#define SRC_REG		 0b00000111

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
	Cmp,

	Jz,
	Jnz,
	Jg,
	Js,
	Ip,
	Not,

	Call,
	Enter,
	Leave,
	Push,
	Pop,
	Halt,
	Syscall,
	Printf,
	Outi,
	Outs,
	Memfind,
	Memset,
	Memcpy,
	Memcmp
};
typedef struct NanoVMCpu{
	uint64_t registers[10];
	unsigned char* codeBase;
	unsigned char* stackBase;
	uint64_t codeSize;
	uint64_t stackSize;
};

typedef struct Instruction {
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

class NanoVM {
public:
	NanoVM(unsigned char* code, uint64_t size);
	NanoVM(std::string file);
	~NanoVM();
	bool Run();
private:
	bool fetch(Instruction &instruction);
	bool execute(Instruction &instruction);
	NanoVMCpu cpu;
};