// NanoDebugger.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

int main(int argc, char *argv[])
{
	std::string file;
	if (argc < 2)
		file = "arithmetic.bin";
	else
		file = (argv[1]);
	NanoDebugger debugger(file);
	debugger.debug();
	return 0;
}