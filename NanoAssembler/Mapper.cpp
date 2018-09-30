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
	opcodeMap["cmp"]	= std::make_pair(10, 2);

	opcodeMap["jz"]		= std::make_pair(11, 1);
	opcodeMap["jnz"]	= std::make_pair(12, 1);
	opcodeMap["jg"]		= std::make_pair(13, 1);
	opcodeMap["js"]		= std::make_pair(14, 1);
	opcodeMap["ip"]		= std::make_pair(15, 1);
	opcodeMap["not"]	= std::make_pair(16, 1);

	opcodeMap["call"]	= std::make_pair(17, 1);
	opcodeMap["enter"]	= std::make_pair(18, 0);
	opcodeMap["leave"]  = std::make_pair(19, 0);
	opcodeMap["push"]	= std::make_pair(20, 1);
	opcodeMap["pop"]	= std::make_pair(21, 1);
	opcodeMap["halt"]	= std::make_pair(22, 0);
	opcodeMap["syscall"]= std::make_pair(23, 1);
	opcodeMap["printf"] = std::make_pair(24, 1);
	opcodeMap["outi"]	= std::make_pair(25, 1);
	opcodeMap["outs"]	= std::make_pair(26, 1);
	opcodeMap["memfind"]= std::make_pair(27, 1);
	opcodeMap["memset"] = std::make_pair(28, 1);
	opcodeMap["memcpy"] = std::make_pair(29, 1);
	opcodeMap["memcmp"] = std::make_pair(30, 1);
}

Mapper::~Mapper() {

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

bool Mapper::mapOpcode(std::string opcodeName, std::pair<unsigned char, unsigned int>& opcode) {
	try {
		opcode = opcodeMap[opcodeName];
		return true;
	}
	catch (std::out_of_range) {
		return false;
	}
}

int Mapper::mapImmediate(std::string value, std::vector<unsigned char>& bytes) {
	try {
		uint64_t value64 = std::stoull(value);
		if (value64 <= UINT8_MAX) {
			bytes.push_back(static_cast<uint8_t>(value64));
			return Byte;
		}
		else if (value64 <= UINT16_MAX) {
			uint16_t value16 = static_cast<uint16_t>(value64);
			bytes.push_back(static_cast<uint8_t>(value16 >> 8));
			bytes.push_back(static_cast<uint8_t>(value16));
			return Short;
		}
		else if (value64 <= UINT32_MAX) {
			uint32_t value32 = static_cast<uint16_t>(value64);
			bytes.push_back(static_cast<uint8_t>(value32 >> 24));
			bytes.push_back(static_cast<uint8_t>(value32 >> 16));
			bytes.push_back(static_cast<uint8_t>(value32 >> 8));
			bytes.push_back(static_cast<uint8_t>(value32));
			return Dword;
		}
		else {
			bytes.push_back(static_cast<uint8_t>(value64 >> 56));
			bytes.push_back(static_cast<uint8_t>(value64 >> 48));
			bytes.push_back(static_cast<uint8_t>(value64 >> 40));
			bytes.push_back(static_cast<uint8_t>(value64 >> 32));
			bytes.push_back(static_cast<uint8_t>(value64 >> 24));
			bytes.push_back(static_cast<uint8_t>(value64 >> 16));
			bytes.push_back(static_cast<uint8_t>(value64 >> 8));
			bytes.push_back(static_cast<uint8_t>(value64));
			return Qword;
		}
	}
	catch (std::invalid_argument) {
		return -1;
	}
	catch (std::out_of_range) {
		return -2;
	}
}