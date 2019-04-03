#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include "NanoAssembler.h"

/**
 * \brief Handles mapping of text representation of instruction parts to their corresponding structures
 *
 * Mapper implements handling for text representation of insturction parts and calculation of sizes for instructions
 * containing relative addresses
*/
class Mapper {
public:

	/**
	 * Initializes Mapper
	*/
	Mapper();

	/**
	 * Mapper destructor
	*/
	~Mapper();

	/**
	 * Maps text representation of opcode to instruction struct
	 * @param opcodeName Text representation of the opcode
	 * @param[out] instruction Instruction struct reference to update with opcode value
	 * @return True if opcode was resolved, false if the opcode was unknown
	*/
	bool mapOpcode(std::string opcodeName, Instruction& instruction);


	bool mapRegister(std::string regName, unsigned char& reg);
	int mapImmediate(std::string value, unsigned char* bytes, unsigned int& length);
	bool canMapLabel(std::string label, unsigned int instructionIndex, std::unordered_map<std::string, unsigned int> labelMap, std::vector<Instruction> instructions);
	int calculateSizeRequirement(std::string label, unsigned int instructionIndex, std::unordered_map<std::string, unsigned int> labelMap, std::vector<Instruction> instructions);
	unsigned int mapLabel(std::string label, unsigned int instructionIndex, std::unordered_map<std::string, unsigned int> labelMap, std::vector<Instruction> &instructions, int64_t &value);
	int mapInteger(int64_t value64, unsigned char* bytes, unsigned int &length);
	template<typename T> void mapImmediate(unsigned char *bytes, T value);

private:
	std::unordered_map<std::string, std::pair<unsigned char, unsigned int>> opcodeMap;
	std::unordered_map<std::string, unsigned char> registerMap;
};