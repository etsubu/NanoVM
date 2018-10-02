#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include "NanoAssembler.h"

class Mapper {
public:
	Mapper();
	~Mapper();
	bool mapOpcode(std::string, Instruction&);
	bool mapRegister(std::string, unsigned char&);
	int mapImmediate(std::string, unsigned char*, unsigned int&);
	int64_t mapLabel(std::string label, unsigned int instructionIndex, std::unordered_map<std::string, unsigned int> labelMap, std::vector<std::vector<unsigned char>> bytecode);
private:
	std::unordered_map<std::string, std::pair<unsigned char, unsigned int>> opcodeMap;
	std::unordered_map<std::string, unsigned char> registerMap;
};