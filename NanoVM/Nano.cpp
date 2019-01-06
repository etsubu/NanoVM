#include "pch.h"


int main(int argc, char* argv[])
{
	if (argc <= 1) {
		std::cout << "Usage NanoVM.exe [FILE]" << std::endl;
		return 0;
	}
	NanoVM vm(argv[1]);
	vm.Run();
	vm.printStatus();
	system("pause");
	return 0;
}