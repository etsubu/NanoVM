#include "pch.h"
#include "Mapper.h"

enum Size {
	Byte = 0b00000000,
	Short = 0b00100000,
	Dword = 0b01000000,
	Qword = 0b01100000
};

Mapper::Mapper() {
	registerMap["reg0"] = 0x00;
	registerMap["reg1"] = 0x01;
	registerMap["reg2"] = 0x02;
	registerMap["reg3"] = 0x03;
	registerMap["reg4"] = 0x04;
	registerMap["reg5"] = 0x05;
	registerMap["reg6"] = 0x06;
	registerMap["esp"]  = 0x07;

	opcodeMap["mov"]	= std::make_pair(0, 2);
	opcodeMap["add"]	= std::make_pair(1, 2);
	opcodeMap["sub"]	= std::make_pair(2, 2);
	opcodeMap["and"]	= std::make_pair(3, 2);
	opcodeMap["or"]		= std::make_pair(4, 2);
	opcodeMap["xor"]	= std::make_pair(5, 2);
	opcodeMap["sar"]	= std::make_pair(6, 2);
	opcodeMap["sal"]	= std::make_pair(7, 2);
	opcodeMap["ror"]	= std::make_pair(8, 2);
	opcodeMap["rol"]	= std::make_pair(9, 2);
	opcodeMap["mul"]    = std::make_pair(10, 2);
	opcodeMap["div"]    = std::make_pair(11, 2);
	opcodeMap["divi"]   = std::make_pair(12, 2);
	opcodeMap["cmp"]	= std::make_pair(13, 2);

	opcodeMap["jz"]		= std::make_pair(14, 1);
	opcodeMap["jnz"]	= std::make_pair(15, 1);
	opcodeMap["jg"]		= std::make_pair(16, 1);
	opcodeMap["js"]		= std::make_pair(17, 1);
	opcodeMap["not"]	= std::make_pair(18, 1);
	opcodeMap["inc"]	= std::make_pair(19, 1);
	opcodeMap["dec"]	= std::make_pair(20, 1);
	opcodeMap["ret"]	= std::make_pair(21, 1);

	opcodeMap["call"]	= std::make_pair(22, 1);
	opcodeMap["push"]	= std::make_pair(23, 1);
	opcodeMap["pop"]	= std::make_pair(24, 1);
	opcodeMap["halt"]	= std::make_pair(25, 0);
	opcodeMap["printi"]	= std::make_pair(26, 1);
	opcodeMap["prints"]	= std::make_pair(27, 1);
	opcodeMap["memfind"]= std::make_pair(28, 1);
	opcodeMap["memset"] = std::make_pair(29, 1);
	opcodeMap["memcpy"] = std::make_pair(30, 1);
	opcodeMap["memcmp"] = std::make_pair(31, 1);
}

Mapper::~Mapper() {

}

int64_t Mapper::mapLabel(std::string label, unsigned int instructionIndex, std::unordered_map<std::string, unsigned int> labelMap, std::vector<std::vector<unsigned char>> bytecode) {
	unsigned int labelIndex;
	try {
		labelIndex = labelMap[label];
	}
	catch (std::out_of_range) {
		return 0;
	}
	if (labelIndex == instructionIndex) {
		// This will cause infinite loop
		return 0;
	}
	if (labelIndex > instructionIndex) {
		int64_t delta = sizeof(uint64_t) + 2; // assume max length for the jump instruction (10 bytes)
		for (unsigned int i = instructionIndex + 1; i < labelIndex; i++) {
			size_t instructionLength = bytecode[i].size();
			int unAssembled = 0;
			if (instructionLength == 0) {
				// Instruction hasn't been assembled yet because it contains label that hasn't been resolved, thus it's length is unknown
				unAssembled++;
				return 0;
			}
			else {
				delta += instructionLength;
			}
			if (unAssembled) {
				// Calculate min and max values for delta
				size_t min = delta + (unAssembled * 2);
				size_t max = delta + (unAssembled * (2 + sizeof(uint64_t)));
				if (min <= SCHAR_MAX && max <= SCHAR_MAX) {
					// 
				}
			}
		}
	}
}

bool Mapper::mapRegister(std::string regName, unsigned char& reg) {
	try {
		reg = registerMap.at(regName);
		return true;
	}
	catch (std::out_of_range) {
		return false;
	}
}

bool Mapper::mapOpcode(std::string opcodeName, Instruction &instruction) {
	try {
		std::pair<unsigned char, unsigned int> opcode = opcodeMap[opcodeName];
		instruction.opcode = opcode.first;
		instruction.operands = opcode.second;
		return true;
	}
	catch (std::out_of_range) {
		return false;
	}
}

int Mapper::mapImmediate(std::string value, unsigned char* bytes, unsigned int &length) {
	if (value.empty())
		return -1;
	try {
		if (value[0] == '-')
		{
			int64_t value64 = std::stoll(value);
			if (value64 <= INT8_MAX) {
				bytes[0] = static_cast<int8_t>(value64);
				length = sizeof(int8_t);
				return Byte;
			}
			else if (value64 <= INT16_MAX) {
				uint16_t value16 = static_cast<int16_t>(value64);
				bytes[0] = (static_cast<int8_t>(value16 >> 8));
				bytes[1] = (static_cast<int8_t>(value16));
				length = sizeof(int16_t);
				return Short;
			}
			else if (value64 <= INT32_MAX) {
				uint32_t value32 = static_cast<int32_t>(value64);
				bytes[0] = (static_cast<int8_t>(value32 >> 24));
				bytes[1] = (static_cast<int8_t>(value32 >> 16));
				bytes[2] = (static_cast<int8_t>(value32 >> 8));
				bytes[3] = (static_cast<int8_t>(value32));
				length = sizeof(int32_t);
				return Dword;
			}
			else {
				bytes[0] = (static_cast<int8_t>(value64 >> 56));
				bytes[1] = (static_cast<int8_t>(value64 >> 48));
				bytes[2] = (static_cast<int8_t>(value64 >> 40));
				bytes[3] = (static_cast<int8_t>(value64 >> 32));
				bytes[4] = (static_cast<int8_t>(value64 >> 24));
				bytes[5] = (static_cast<int8_t>(value64 >> 16));
				bytes[6] = (static_cast<int8_t>(value64 >> 8));
				bytes[7] = (static_cast<int8_t>(value64));
				length = sizeof(int64_t);
				return Qword;
			}
		}
		else {
			uint64_t value64 = std::stoull(value);
			if (value64 <= UINT8_MAX) {
				bytes[0] = (static_cast<uint8_t>(value64));
				length = sizeof(uint8_t);
				return Byte;
			}
			else if (value64 <= UINT16_MAX) {
				uint16_t value16 = static_cast<uint16_t>(value64);
				bytes[0] = (static_cast<uint8_t>(value16 >> 8));
				bytes[1] = (static_cast<uint8_t>(value16));
				length = sizeof(uint16_t);
				return Short;
			}
			else if (value64 <= UINT32_MAX) {
				uint32_t value32 = static_cast<uint32_t>(value64);
				bytes[0] = (static_cast<uint8_t>(value32 >> 24));
				bytes[1] = (static_cast<uint8_t>(value32 >> 16));
				bytes[2] = (static_cast<uint8_t>(value32 >> 8));
				bytes[3] = (static_cast<uint8_t>(value32));
				length = sizeof(uint32_t);
				return Dword;
			}
			else {
				bytes[0] = (static_cast<uint8_t>(value64 >> 56));
				bytes[1] = (static_cast<uint8_t>(value64 >> 48));
				bytes[2] = (static_cast<uint8_t>(value64 >> 40));
				bytes[3] = (static_cast<uint8_t>(value64 >> 32));
				bytes[4] = (static_cast<uint8_t>(value64 >> 24));
				bytes[5] = (static_cast<uint8_t>(value64 >> 16));
				bytes[6] = (static_cast<uint8_t>(value64 >> 8));
				bytes[7] = (static_cast<uint8_t>(value64));
				length = sizeof(uint64_t);
				return Qword;
			}
		}
	}
	catch (std::invalid_argument) {
		return -1;
	}
	catch (std::out_of_range) {
		return -2;
	}
}