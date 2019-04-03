#include "pch.h"

int main(int argc, char *argv[])
{
	if (argc < 1) {
		std::cout << "Usage NanoDebugger.exe [FILE]" << std::endl;
	}
	std::string file = (argv[1]);
	NanoDebugger debugger(file);
	debugger.debug();
	return 0;
}