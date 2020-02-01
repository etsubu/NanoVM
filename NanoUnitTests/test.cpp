#include "../NanoAssembler/NanoAssembler.h"
#include "../NanoVM/NanoVM.h"
#include <fstream>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

/**
 * This file contains unit tests for NanoVM + NanoAssembler
 * The tests load up Nano assembler files (.nano file extension), assemble those to binary files, run those
 * and verify the results.
 * This way we do not bind the unit tests to bytecode format but rather enforce that the NanoVM bytecode
 * is executed properly and does not contain bugs while allowing us to modify the assembler
 * without breaking the tests
*/

int runSingleTest(NanoAssembler& assembler, std::string& path) {
	unsigned char* bytecode;
	unsigned int length;
	AssemblerReturnValues ret = assembler.assembleToMemory(path, bytecode, length);
	// Assembling should succeed
	if (ret != AssemblerReturnValues::Success)
		return 1;
	// Read the assembly file
	std::string expectedReturnKey = "NANO_TEST_EXPECT_RETURN=";
	std::ifstream file(path, std::ios::in | std::ios::ate);
	int expectedValue;
	if (file.is_open())
	{
		unsigned long size = file.tellg();
		char *memblock = new char[size + 1];
		file.seekg(0, std::ios::beg);
		file.read(memblock, size);
		file.close();
		memblock[size] = '\0';
		std::string content(memblock);
		delete[] memblock;
		int index = content.find(expectedReturnKey);
		if (index == -1 || index == content.length() - expectedReturnKey.length()) {
			// Not a test file
			return 0;
		}
		std::string expectedReturnStr = content.substr(index + expectedReturnKey.length());
		try {
			expectedValue = std::stoi(expectedReturnStr);
		}
		catch (std::invalid_argument & e) {
			return 3;
		}
		catch (std::out_of_range & e) {
			return 3;
		}
	}
	else {
		return 4;
	}
	// Fire up the VM
	NanoVM vm(bytecode, length);
	int vmValue = vm.Run();
	if (vmValue == expectedValue) {
		std::cout << "Test passed: " << path.substr(path.find_last_of("/")) << std::endl;
		return 0;
	}
	std::cout << "Test failed: " << path.substr(path.find_last_of("/")) << " Expected value: " << expectedValue << " but was " << vmValue << std::endl;
	return 5;
}

int runTests() {
	NanoAssembler assembler;
	std::string path = "../../../../examples";
	std::string ending = ".nano";
	int totalTests = 0;
	int failedTests = 0;
	for (const auto& entry : fs::directory_iterator(path)) {
		std::string path = entry.path().string();
		std::cout << path << std::endl;
		if (path.compare(path.length() - ending.length(), ending.length(), ending) == 0) {
			// Run only test for files with .nano ending
			int status = runSingleTest(assembler, path);
			if (status) {
				failedTests++;
			}
			totalTests++;
		}
	}
	if (!failedTests) {
		// All available tests passed
		std::cout << "All tests passed! " << totalTests << "/" << totalTests << std::endl;
		return 0;
	}
	std::cout << "Failed tests " << failedTests << " / " << totalTests << std::endl;
	return 1;
}

// main
int main(int argc, char* argv[]) {
	runTests();
	return 0;
}