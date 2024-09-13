#pragma once
#include <iostream>
#include <cstdint>

constexpr uint8_t SRC_TYPE = 0b10000000;
constexpr uint8_t SRC_SIZE = 0b01100000;
constexpr uint8_t DST_MEM =  0b00010000;
constexpr uint8_t SRC_MEM =  0b00001000;

#ifndef TYPE_H
#define TYPE_H

/**
 * DataType enum holds the type mask of value reg/immediate
*/
enum DataType {
	Reg = 0,
	Immediate = 0b10000000
};

enum Size {
	Byte = 0b00000000,
	Short = 0b00100000,
	Dword = 0b01000000,
	Qword = 0b01100000
};
#endif

/**
 * Instruction represent a single instruction to be assembled
*/
struct AssemberInstruction {
	std::string line;
	unsigned char bytecode[2 + sizeof(int64_t)];
	unsigned char opcode;
	unsigned int operands;
	unsigned int length;
	unsigned int lineNumber;
	bool assembled;
};
typedef struct AssemberInstruction AssemberInstruction;

enum AssemblerReturnValues {
	Success,
	AssemblerError,
	MemoryAllocationError,
	IOError
};