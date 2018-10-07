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
	bool canMapLabel(std::string label, unsigned int instructionIndex, std::unordered_map<std::string, unsigned int> labelMap, std::vector<Instruction> instructions);
	int calculateSizeRequirement(std::string label, unsigned int instructionIndex, std::unordered_map<std::string, unsigned int> labelMap, std::vector<Instruction> instructions);
	unsigned int mapLabel(std::string label, unsigned int instructionIndex, std::unordered_map<std::string, unsigned int> labelMap, std::vector<Instruction> &instructions, int64_t &value);
	int mapInteger(int64_t value64, unsigned char* bytes, unsigned int &length);
	template<typename T> void mapImmediate(unsigned char *bytes, T value);

private:
	std::unordered_map<std::string, std::pair<unsigned char, unsigned int>> opcodeMap;
	std::unordered_map<std::string, unsigned char> registerMap;
};