#include "../NanoAssembler/NanoAssembler.h"
#include "../NanoVM/NanoVM.h"
#include "gtest/gtest.h"

/**
 * This file contains unit tests for NanoVM + NanoAssembler
 * The tests load up Nano assembler files (.nano file extension), assemble those to binary files, run those
 * and verify the results.
 * This way we do not bind the unit tests to bytecode format but rather enforce that the NanoVM bytecode
 * is executed properly and does not contain bugs (at least so many) while allowing us to modify the assembler
 * without breaking the tests
*/
TEST(NanoTest, basicArithmetic) {
	NanoAssembler assembler;
	std::string file = "../../examples/arithmetic.nano";
	unsigned char* bytecode;
	unsigned int length;
	AssemblerReturnValues ret = assembler.assembleToMemory(file, bytecode, length);
	// Assembling should succeed
	EXPECT_EQ(ret, AssemblerReturnValues::Success);
	// Fire up the VM
	NanoVM vm(bytecode, length);
	EXPECT_EQ(vm.Run(), 5);
}

TEST(NanoTest, fibonacciIteration) {
	NanoAssembler assembler;
	std::string file = "../../examples/fibonacciSequence.nano";
	unsigned char* bytecode;
	unsigned int length;
	AssemblerReturnValues ret = assembler.assembleToMemory(file, bytecode, length);
	// Assembling should succeed
	EXPECT_EQ(ret, AssemblerReturnValues::Success);
	// Fire up the VM
	NanoVM vm(bytecode, length);
	EXPECT_EQ(vm.Run(), 4181);
}

TEST(NanoTest, primeNumbers) {
	NanoAssembler assembler;
	std::string file = "../../examples/SieveOfEratosthenes.nano";
	unsigned char* bytecode;
	unsigned int length;
	AssemblerReturnValues ret = assembler.assembleToMemory(file, bytecode, length);
	// Assembling should succeed
	EXPECT_EQ(ret, AssemblerReturnValues::Success);
	// Fire up the VM
	NanoVM vm(bytecode, length);
	EXPECT_EQ(vm.Run(), 293);
}

TEST(NanoTest, loop1) {
	NanoAssembler assembler;
	std::string file = "../../examples/labels.nano";
	unsigned char* bytecode;
	unsigned int length;
	AssemblerReturnValues ret = assembler.assembleToMemory(file, bytecode, length);
	// Assembling should succeed
	EXPECT_EQ(ret, AssemblerReturnValues::Success);
	// Fire up the VM
	NanoVM vm(bytecode, length);
	EXPECT_EQ(vm.Run(), 5);
}

TEST(NanoTest, loop2) {
	NanoAssembler assembler;
	std::string file = "../../examples/labels2.nano";
	unsigned char* bytecode;
	unsigned int length;
	AssemblerReturnValues ret = assembler.assembleToMemory(file, bytecode, length);
	// Assembling should succeed
	EXPECT_EQ(ret, AssemblerReturnValues::Success);
	// Fire up the VM
	NanoVM vm(bytecode, length);
	EXPECT_EQ(vm.Run(), 1);
}

TEST(NanoTest, loop4) {
	NanoAssembler assembler;
	std::string file = "../../examples/labels4.nano";
	unsigned char* bytecode;
	unsigned int length;
	AssemblerReturnValues ret = assembler.assembleToMemory(file, bytecode, length);
	// Assembling should succeed
	EXPECT_EQ(ret, AssemblerReturnValues::Success);
	// Fire up the VM
	NanoVM vm(bytecode, length);
	EXPECT_EQ(vm.Run(), 9);
}

// main
int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	system("pause");
	return RUN_ALL_TESTS();
}