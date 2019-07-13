#pragma once

#include <iostream>
#include <fstream>

constexpr uint32_t PAGE_SIZE	= 4096;
constexpr uint8_t OPCODE_MASK	= 0b00011111;
constexpr uint8_t DST_REG_MASK	= 0b11100000;
constexpr uint8_t SRC_TYPE_MASK	= 0b10000000;
constexpr uint8_t SRC_SIZE_MASK = 0b01100000;
constexpr uint8_t DST_MEM_MASK  = 0b00010000;
constexpr uint8_t SRC_MEM_MASK  = 0b00001000;
constexpr uint8_t SRC_REG_MASK  = 0b00000111;

constexpr uint8_t STACK_ERROR	= 0b10000000;
constexpr uint8_t IP_ERROR		= 0b01000000;
constexpr uint8_t MEMORY_ACCESS = 0b00100000;

constexpr uint8_t ZERO_FLAG		= 0b10000000;
constexpr uint8_t GREATER_FLAG	= 0b01000000;
constexpr uint8_t SMALLER_FLAG	= 0b00100000;

/**
 * Register enum defines all the CPU registers + flags and instruction pointer
*/
enum Register {
	Reg0,
	Reg1,
	Reg2,
	Reg3,
	Reg4,
	Reg5,
	bp, // base pointer for current stack frame
	esp, 
	ip,
	flags
};

/**
 * Opcodes enum defines all the implemented opcodes for the NanoVM. Limited to 5 bits aka 32 opcodes
*/
enum Opcodes {
	Mov,
	Add,
	Sub,
	And,
	Or,
	Xor,
	Sar,
	Sal,
	Ror,
	Rol,
	Mul,
	Div,
	Mod,
	Cmp,

	Jz,
	Jnz,
	Jg,
	Js,
	Jmp,
	Not,
	Inc,
	Dec,
	Ret,

	Call,
	Push,
	Pop,
	Halt,
	Printi,
	Prints,
	Printc,
	Syscall,
	Memcpy
};

#ifndef TYPE_H
#define TYPE_H

/**
 * Size enum defines used integer sizes which are 8, 16, 32, 64 bits
*/
enum Size {
	Byte,
	Short,
	Dword,
	Qword
};

/**
 * DataType enum defines data types for the instructions which can be register or immediate value
*/
enum DataType {
	Reg,
	Immediate
};

#endif // !TYPE_H

/**
 * NanoVMCpu struct defines the CPU core which holds registers and pointers to code base, stack base and their respective sizes
*/
struct NanoVMCpu{
	uint64_t registers[10]; /**< CPU registers + IP and flags */
	unsigned char* codeBase; /**< Pointer to the base of the VM memory */
	unsigned char* stackBase; /**< Pointer to the base of the stack */
	uint64_t codeSize; /**< Size of the allocated VM memory including stack */
	uint64_t stackSize; /**< Size of the allocated stack memory */
	uint64_t bytecodeSize; /**< Size of the loaded bytecode */
};

/**
 * Instruction holds a single fetched instruction which the VM can run
*/
struct Instruction {
	unsigned char opcode; /**< Opcode of the instruction */
	unsigned char dstReg; /**< Destination register which is defined in the 1st byte of instruction with opcode */ 
	unsigned char srcReg; /**< Source register (optional) */
	unsigned char srcType; /**< Source value type reg/immediate (optional) */
	bool isDstMem; /**< Is destination register pointer to memory */
	bool isSrcMem; /**< Is source value pointer to memory */
	unsigned char srcSize; /**< Size of the source value (optional) */
	uint64_t immediate; /**< Immediate value aka source value (optinal) */
	unsigned char instructionSize; /**< Size of this instruction. This allows the vm to adjust the IP accordingly */
};

typedef struct NanoVMCpu NanoVMCpu;
typedef struct Instruction Instruction;

/**
 * \brief NanoVM is the VM core which will load and run nano bytecode
 *
 * NanoVM is the VM core which will load the bytecode, allocate and initialize memory and the CPU.
 * It implements feching and executing of instructions, stack memory handling and running of the bytecode
*/
class NanoVM {
public:
	/**
	 * \brief Initializes the NanoVM from bytecode
	 * @param code Points to the bytecode to be loaded
	 * @param size Holds the size of the bytecode to be loaded
	*/
	NanoVM(unsigned char* code, uint64_t size);

	/**
	 * Initializes the NanoVM from bytecode file
	 * @param file File to load the bytecode from
	*/
	NanoVM(std::string file);

	/**
	 * NanoVM destructor
	*/
	~NanoVM();

	/**
	 * Runs the whole loaded bytecode program
	 * @return Return value of the bytecode program
	*/
	uint64_t Run();
protected:
	/**
	 * Pops a value from the stack and adjusts the stack pointer
	 * @return Single value from the stack
	*/
	template<class T> T pop();

	/**
	 * Pushes a value to the stack
	 * @param value Value to push to the stack
	*/
	template<class T> void push(T value);

	/**
	 * Fetches the next instruction pointed by the instruction pointer (IP). Note that fetch does not check if the instruction is valid
	 * @param[out] Reference to instruction struct to be updated
	 * @return True if instruction was fetched successfully, false if failed (e.g IP out of bounds)
	*/
	bool fetch(Instruction &instruction) const;

	/**
	 * Executes a single instruction and updates the internal state of the VM including IP
	 * @param instruction Instruction to be executed
	 * @return True if the instruction was executed successfully, false if the instruction was not valid or an error occurred
	*/
	bool execute(Instruction &instruction);

	unsigned char errorFlag; /**< 8 bit flag that will be set with error masks if an error occurs */
	NanoVMCpu cpu; /**< Holds the internal state of the CPU */
};