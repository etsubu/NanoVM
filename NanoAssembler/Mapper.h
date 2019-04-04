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

	/**
	 * Maps a text representation of register to its corresponding register value
	 * @param[out] reg Reference to the value to hold the resolved register value
	 * @return True if register name was resolved, false if the name was unknown
	*/
	bool mapRegister(std::string regName, unsigned char& reg);

	/**
	 * Maps a text representation of immediate value to bytes
	 * @param value Text representation of integer
	 * @param[out] bytes Pointer to array to hold the bytes of the integer
	 * @param[out] length Reference to integer to hold the amount of bytes of the resolved immediate value
	*/
	int mapImmediate(std::string value, unsigned char* bytes, unsigned int& length);

	/**
	 * Checks if the given text label can be resolved as relative address from the given instruction
	 * @param label Name of the label to try mapping to address
	 * @param instructionIndex The index of the instruction we try to resolve the relative label address from
	 * @param labelMap Map structure holding all labels
	 * @param instructions List of all the instructions
	 * @return True if the label can be resolved to relative address from the current instruction, false if not
	*/
	bool canMapLabel(std::string label, unsigned int instructionIndex, std::unordered_map<std::string, unsigned int> labelMap, std::vector<Instruction> instructions);
	
	/**
	 * Calculate the size requirement in bytes for the relative label address from the current instruction.
	 * Should be called only if canMapLabel() returns true
     * @param label Name of the label to try mapping to address
	 * @param instructionIndex The index of the instruction we try to resolve the relative label address from
	 * @param labelMap Map structure holding all labels
	 * @param instructions List of all the instructions
	 * @return Size of the relative address in bytes
	*/
	int calculateSizeRequirement(std::string label, unsigned int instructionIndex, std::unordered_map<std::string, unsigned int> labelMap, std::vector<Instruction> instructions);

	/**
	 * Maps label to relative address from the current instruction. Should noly be called if canMapLabel returns true
	 * @param label Name of the label to try mapping to address
	 * @param instructionIndex The index of the instruction we try to resolve the relative label address from
	 * @param labelMap Map structure holding all labels
	 * @param[out] instructions[out] Reference to the list of all the instructions. Corresponding instructions will be updated with the relative address
	 * @return Size of the resolved relative address
	*/
	unsigned int mapLabel(std::string label, unsigned int instructionIndex, std::unordered_map<std::string, unsigned int> labelMap, std::vector<Instruction> &instructions, int64_t &value);
	
	/**
	 * Maps integer to bytes with minimum required bytes
	 * @param value64 64-bit signed integer representing the value to be mapped in bytes
	 * @param[out] bytes Pointer to array that will be updated with the integer bytes
	 * @param[out] length Reference that will hold the number of bytes that were stored in the array
	 * @return Size mask for the instruction bytes to use for setting the size of immediate value
	*/
	int mapInteger(int64_t value64, unsigned char* bytes, unsigned int &length);

	/**
	 * Copies given integer to byte array as bytes
	 * @param bytes Pointer to array to be updated with the integer bytes
	 * @param value Integer value to be copied
	*/
	template<typename T> void mapImmediate(unsigned char *bytes, T value);

private:
	std::unordered_map<std::string, std::pair<unsigned char, unsigned int>> opcodeMap; /**< Map between all the opcodes text representation and corresponding values */
	std::unordered_map<std::string, unsigned char> registerMap; /**< Map between register text representations and register number */
};