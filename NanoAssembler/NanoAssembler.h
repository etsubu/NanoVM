#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <algorithm>
#include <sstream>
#include "Types.h"
#include "Mapper.h"

/**
 * NanoAssembler instance handles loading assembler files and compiling those to bytecode format
*/
class NanoAssembler {
public:
	NanoAssembler();
	~NanoAssembler();

	/**
	 * \brief Assembles input file and writes the resulting bytecode to a file on disk
	 *
	 * NanoAssembler loads the input file containing assembler instructions, compiles those in to binary format
	 * and writes them to a binary file on disk
	 * @param inputFile Points to the assembler file to load
	 * @param outputFile Points to the file where the compiled bytecode will be written
	 * @return 1 on success and anything else meaning failure
	 */
	AssemblerReturnValues assembleToFile(std::string inputFile, std::string outputFile);

	/**
	 * \brief Assembles input file to buffer in memory
	 *
	 * NanoAssembler loads the input file containing assembler instructions, compiles those in to binary format
	 * and outputs them to dynamically allocated buffer
	 * @param inputFile Points to the assembler file to load
	 * @param[out] bytecodeBuffer Reference to a pointer that will point to the compiled bytecode buffer
	 * @param[out] size Reference to an int that will hold the size of the bytecodeBuffer in bytes
	 * @return 1 on success and anything else meaning failure
	*/
	AssemblerReturnValues assembleToMemory(std::string inputFile, unsigned char*& bytecodeBuffer, unsigned int &size);
private:
	bool readLines(std::string file, std::vector<AssemberInstruction>& lines, std::unordered_map<std::string, size_t>& labelMap);
	int assembleInstruction(int i, std::vector<AssemberInstruction>& instructionBytes, std::unordered_map<std::string, size_t> labelMap, bool initial);
	bool assemble(std::vector<AssemberInstruction>& instruction, std::unordered_map<std::string, size_t>& labelMap);

	Mapper mapper;
};