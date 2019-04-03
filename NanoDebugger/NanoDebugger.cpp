#include "pch.h"
#include "NanoDebugger.h"

NanoDebugger::NanoDebugger(std::string file) : NanoVM(file) {
	run = false;
}

NanoDebugger::NanoDebugger(unsigned char *bytecode, uint64_t size) : NanoVM(bytecode, size) {
	run = false;
}

NanoDebugger::~NanoDebugger() {

}

bool NanoDebugger::disassembleInstruction(std::string &instruction) {
	Instruction ins;
	if (!fetch(ins)) {
		return false;
	}
	std::string opcode = instructionStr[ins.opcode];
	if (ins.opcode == Opcodes::Halt || ins.opcode == Opcodes::Ret) {
		instruction = opcode;
		return true;
	}
	// Single param instructions
	if (ins.opcode == Opcodes::Jg || ins.opcode == Opcodes::Js || ins.opcode == Opcodes::Jnz || ins.opcode == Opcodes::Jz ||
		ins.opcode == Opcodes::Jmp || ins.opcode == Opcodes::Push || ins.opcode == Opcodes::Pop || ins.opcode == Opcodes::Call ||
		ins.opcode == Opcodes::Dec || ins.opcode == Opcodes::Inc || ins.opcode == Opcodes::Printc || ins.opcode == Opcodes::Printi ||
		ins.opcode == Opcodes::Prints) {
		if (ins.srcType == Type::Reg) {
			instruction = opcode + ((ins.isSrcMem) ? " @reg" : " reg") + std::to_string(ins.srcReg);
		}
		else {
			instruction = opcode + ((ins.isSrcMem) ? " @" : " ") + std::to_string(ins.immediate);
		}
	}
	// two param instruction 
	else {
		if (ins.srcType == Type::Reg) {
			instruction = opcode + ((ins.isDstMem) ? " @reg" : " reg") + std::to_string(ins.dstReg) + ", " +
				((ins.isSrcMem) ? " @reg" : "reg") + std::to_string(ins.srcReg);
		}
		else {
			instruction = opcode + ((ins.isDstMem) ? " @reg" : " reg") + std::to_string(ins.dstReg) + ", " +
				((ins.isSrcMem) ? "@" : "") + std::to_string(ins.immediate);
		}
	}
	return true;
}

bool NanoDebugger::handleInteractive() {
	int value = 0;
	do {
		std::string instruction;
		if (!disassembleInstruction(instruction)) {
			std::cout << "Failed to fetch instruction: IP out of bounds! IP: " << cpu.registers[ip] << std::endl;
			return false;
		}
		std::cout << cpu.registers[ip] << ". " << instruction << std::endl;
		std::cout << "> ";
		value = _getch();
		std::cout << "\b\b";
		if (value == 'h') {
			std::cout << "\n(s)tack\nr(e)gisters\n(b)reakpoint\n(r)un\n(c)lean breakpoint" << std::endl;
		}
		else if (value == 'e') {
			std::cout << "\nRegisters:\n";
			for (int i = 0; i < 8; i++) {
				std::cout << "reg" << i << ": " << cpu.registers[i] << std::endl;
			}
		}
		else if (value == 'r') {
			run = true;
			return true;
		}
		else if (value == 'b') {
			std::cout << "Breakpoint where (offset): ";
			int offset;
			std::cin >> offset;
			breakpoints.insert(offset);
		}
		else if (value == 'c') {
			auto a = breakpoints.find(cpu.registers[ip]);
			if (a == breakpoints.end()) {
				std::cout << "No breakpoint was placed here!" << std::endl;
			}
			else {
				breakpoints.erase(a);
				std::cout << "Breakpoint removed!" << std::endl;
			}
		}
		else if (value == 's') {
			printStack();
		}
	} while (value != 13);
	return true;
}

void NanoDebugger::printStack() {
	int counter = 0;
	unsigned char *p = (cpu.stackBase);
	uint64_t size = (cpu.registers[esp] + cpu.codeBase) - cpu.stackBase;
	std::cout << "\nStack size: " << size << "\n";
	for (int i = 0; i < size; i++) {
		if (counter == 7) {
			counter = 0;
			std::printf("%02X | %c %c %c %c %c %c %c %c\n", p[i], p[i - 7], p[i - 6], p[i - 5], p[i - 4], p[i - 3], p[i - 2], p[i - 1], p[i]);
			continue;
		}
		else {
			std::printf("%02X ", p[i]);
		}
		counter++;
	}
	if (counter) {
		std::printf("|  ");
		for (int i = counter; i > 0; i--) {
			std::printf("%c  ", p[size - i]);
		}
		std::printf("\n");
	}
	std::printf("\n");
}

bool NanoDebugger::debug() {
	run = false;
	while (cpu.registers[ip] < cpu.bytecodeSize) {
		Instruction inst;
		if (fetch(inst)) {
			if (breakpoints.find(cpu.registers[ip]) != breakpoints.end()) {
				std::cout << "Breakpoint triggered! " << cpu.registers[ip] << std::endl;
				run = false;
				handleInteractive();
			}
			else if (!run) {
				handleInteractive();
			}
			if (inst.opcode == Halt) {
				std::cout << "VM halted!" << std::endl;
				handleInteractive();
				break;
			}
			if (!execute(inst)) {
				switch (errorFlag) {
				case MEMORY_ACCESS:
					std::cout << "Tried to read/write memory outside of VM!" << std::endl;
					break;
				default:
					std::cout << "Unknown error!" << std::endl;
				}
				return false;
			}
		}
		else {
			std::cout << "Invalid instruction!" << std::endl;
			return false;
		}
	}
	std::cout << "VM exited with return code: " << cpu.registers[Reg0] << std::endl;
	handleInteractive();
	return true;
}