#pragma once
#include "../NanoVM/NanoVM.h"
#include "Instructions.h"
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <fstream>
#include <conio.h>

class NanoDebugger : NanoVM {
public:
	NanoDebugger(std::string file);
	NanoDebugger(unsigned char *bytecode, uint64_t size);
	~NanoDebugger();
	bool debug();
	//bool disassembleToFile(std::string out);
private:
	bool disassembleInstruction(std::string &instruction);
	void printStack();
	bool handleInteractive();
	std::set<uint64_t> breakpoints;
	bool run;
};