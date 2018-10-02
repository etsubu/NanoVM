#pragma once

#define SRC_TYPE 0b10000000
#define SRC_SIZE 0b01100000
#define SRC_MEM  0b00010000
#define DST_MEM  0b00001000

enum Type {
	Reg = 0,
	Immediate = 0b10000000
};

typedef struct Instruction {
	std::string line;
	unsigned char bytecode[2 + sizeof(int64_t)];
	unsigned char opcode;
	unsigned int operands;
	unsigned int length;
	unsigned int lineNumber;
	bool assembled;
};