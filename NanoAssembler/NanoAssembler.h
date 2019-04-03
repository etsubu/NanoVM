#pragma once

constexpr uint8_t SRC_TYPE = 0b10000000;
constexpr uint8_t SRC_SIZE = 0b01100000;
constexpr uint8_t DST_MEM  = 0b00010000;
constexpr uint8_t SRC_MEM  = 0b00001000;

/**
 * Type enum holds the type mask of value reg/immediate
*/
enum Type {
	Reg = 0,
	Immediate = 0b10000000
};

/**
 * Instruction represent a single instruction to be assembled
*/
typedef struct Instruction {
	std::string line;
	unsigned char bytecode[2 + sizeof(int64_t)];
	unsigned char opcode;
	unsigned int operands;
	unsigned int length;
	unsigned int lineNumber;
	bool assembled;
};