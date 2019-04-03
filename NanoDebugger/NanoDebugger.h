#pragma once
#include "../NanoVM/NanoVM.h"
#include "Instructions.h"
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <fstream>
#include <conio.h>

/**
 * \brief NanoDebugger inherits NanoVM allowing more control over the execution of the program
 *
 * NanoDebugger inherits NanoVM implementation and allows to step through the execution, dump stack, set breakpoints
 * disassembling of instructions and other debugger behavior. 
*/
class NanoDebugger : NanoVM {
public:
	/**
	 * Initializes NanoDebugger
	 * @param file Bytecode file to load
	*/
	NanoDebugger(std::string file);

	/**
	 * Initializes NanoDebugger
	 * @param bytecode Bytecode buffer to load
	 * @param size Size of the bytecode buffer
	*/
	NanoDebugger(unsigned char *bytecode, uint64_t size);

	/**
	 * NanDebugger destructor
	*/
	~NanoDebugger();

	/**
	 * Starts interactive debugging of the loaded bytecode program
	 * @return True if bytecode program was executed successfully, false if error occurred
	*/
	bool debug();
	//bool disassembleToFile(std::string out);
private:

	/**
	 * Disassembles the next instruction pointed by IP
	 * @param[out] instruction String reference to hold the text representation of disassembled instruction
	 * @return True if instruction was disassembled successfully, false if failed
	*/
	bool disassembleInstruction(std::string &instruction);

	/**
	 * Prints stack dump of the stack memory on screen
	*/
	void printStack();

	/**
	 * Handles interactive mode for the current instruction allowing user to interact with the program
	 * @return True if successfull, false if failed
	*/
	bool handleInteractive();

	std::set<uint64_t> breakpoints; /**< Set of all active breakpoints */
	bool run; /**< Boolean value whether to run until breakpoint is hit or false if stepping through */
};