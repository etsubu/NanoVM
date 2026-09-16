# NanoVM
Embeddable lightweight x64 register based VM implementation written in C.

### Table of contents

- [NanoVM](#nanovm)
  * [General](#general)
  * [How to build](#how-to-build)
    + [Windows (Visual Studio 2019)](#windows--visual-studio-2019-)
    + [Debian](#debian)
  * [VM architecture](#vm-architecture)
    + [Registers](#registers)
    + [Instructions](#instructions)
- [NanoAssembler](#nanoassembler)
- [NanoDebugger](#nanodebugger)

## General 

NanoVM is cross-platform register based VM with stack memory and heap memory. The goal of the vm core is to be small and embeddable while staying close to POSIX compliant allowing to target different platforms and architectures like webasm for example. Performance is considered opportunistically but is less of priority.

Note that the project is a work in progress and bytecode format, opcodes or any part is subject to change without any backwards compatibility.

## Getting started

Start with [build instructions](#how-to-build)

See examples/ folder for example bytecode programs used as test cases.

Assemble test program to bytecode and run it:
```
./NanoAssembler examples/fibonacciSequence.nano
./NanoVM examples/fibonacciSequence.nanoc
```

Run the program with debugger:
```
./NanoDebugger examples/fibonacciSequence.nanoc
```


## How to build

Build instructions have been tested on Windows and Debian based linux distros

### Windows (Visual Studio)

You need to have Visual Studio and cmake installed on your system.\
Visual Studio is compatible with cmake projects so you can build the project by opening the project in visual studio, right click the root CMakeLists.txt -> "Generate Cache for NanoVM". This will generate the cmake cache for you and now you can build the project by selecting 
from the menu bar: Build -> Build all.\
If you rather wish to generate visual studio specific build files you can do that by running the following command in the project root with cmd/powershell:

```
cmake . -B ./build
```

This will generate new Visual Studio build files under build/

### Debian

You need to have build tools and cmake available. You can install those by running the following commands in terminal 
```
sudo apt install build-essentials
sudo apt install cmake
```
Now to build the project run the following commands
```
git clone https://github.com/etsubu/NanoVM.git
cd NanoVM
cmake .
make
```
This will build all the binaries in their own folders along the source files.


### Assembler build

Assembler is written in go so go needs to be installed https://go.dev/doc/install

```
cd GoAssembler
go build -o NanoAssembler ./cmd
```

## VM architecture

The VM memory are defined as pages which by default are 4096 bytes each. When initialized the VM bytecode will be placed at the bottom of the allocated memory followed by the stack memory and finally heap memory. This allows to grow heap memory dynamically if needed.

NanoVM memory follows Little Endian (LSB) encoding.

### Memory layout

        Higher memory addresses
        ┌─────────────────────┐
        │      HEAP           │
        │                     │
        │         ↑           │
        │      grows up       │
        ├─────────────────────┤
        │      STACK          │
        │                     │
        │         ↑           │
        │      grows up       │
        ├─────────────────────┤
        │     BYTECODE        │
        │                     │
        └─────────────────────┘
        Lower memory addresses

### Registers
The VM registers are encoded with 3 bits so there are 8 registers in total (the names will change in future) and they are 64-bit signed integers.

| Register        | Number        | Description                                  |
| -------------   |:-------------:| --------------------------------------------:|
| Reg0            | 0             | General purpose. Used to store return values |
| Reg1            | 1             | General purpose.                             |
| Reg2            | 2             | General purpose.                             |
| Reg3            | 3             | General purpose.                             |
| Reg4            | 4             | General purpose.                             |
| Reg5            | 5             | General purpose.                             |
| bp              | 6             | Base pointer. Used for stack frames          |
| sp              | 7             | Stack pointer. Points to the top of the stack|

### Instructions
Instructions have always an opcode and 0-2 operands. Below is the instruction encoding defined from LSB to MSB

| 5 bits           | 3 bits                | 1 bit             | 2 bits                      | 1 bits        | 1 bit                  | 3 bits        |
| -------------    |:---------------------:|:-----------------:|:---------------------------:|:-------------:|:----------------------:|:-------------:|
| Opcode           | Destination register  | Source type       | Source size                 | float or int  | reserved               |Source register|
| What instruction | Update this register  | Reg=0, Immediate=1| 8,16,32,64 bit              | 0=int, 1=float|                        | Source register if src type is reg|

So most of the instructions are encoded in 2 bytes + immediate value if used. Instructions that use zero operands effectively being only 1 byte are:
```assembly
Halt ; Stops the execution and exits the VM execution
ret ; Pops value from the top of the stack and performs absolute jump to that address. Updates stack pointer
```
Instructions that use 1 operand encode a register operand in the source register field with source type Reg, and an immediate operand in the immediate value with source type Immediate. The destination register field is unused. Opcodes that use 1 operand:
```assembly
	Jz; Jump if zero flag is set. Example: jz reg0
	Jnz; Jump if zero flag is not set. Example: jnz reg0
	Jg;  Jump if greater flag is set. Example: jg reg0
	Js;  Jump if smaller flag is set. Example: js reg0
	Jmp; Jump ("goto") instruction. Example: jmp reg0
	Not; Flip the bits in value. Example: not reg0
	Call; Pushes the next instructions absolute memory address to the stack and jumps to the given address. Updates stack pointer. Example: call reg0
	Push; Pushes value to the top of the stack. Example: push reg0
	Pop; Pops value from the top of the stack and moves the value to given register. Example: pop reg0
	free; Free heap memory allocation in the given address. Example free reg0
	inc; Increment the register by one and update ZERO_FLAG. Example: inc reg0
	Dec; Decrease the register by one and update ZERO_FLAG. Example: dec reg0
```
Push and pop always move a full 64 bit slot so that call and ret agree on the size of a return address.

An immediate jump or call target is a relative offset measured from the start of the jump instruction itself, not from the instruction that follows it.

Instructions with 2 operands:
```assembly
	Mov; mov reg0, reg1 <=> reg0 = reg1
	Add; add reg0, reg1 <=> reg0 += reg1
	Sub; sub reg0, reg1 <=> reg0 -= reg1
	And; and reg0, reg1 <=> reg0 &= reg1
	Or;  or reg0, reg1 <=> reg0 |= reg1
	Xor; xor reg0, reg1 <=> reg0 ^= reg1
	Shl; shl reg0, reg1 <=> reg0 <<= reg1, shift left
	Shr; shr reg0, reg1 <=> reg0 >>= reg1, shift right
	Mul; mul reg0, reg1 <=> reg0 *= reg1
	Div; div reg0, reg1 <=> reg0 /= reg1
	Mod; mod reg0, reg1 <=> reg0 %= reg1
	Cmp; cmp reg0, reg1 | Compares the 2 values as signed integers and sets comparison flags
	Load; load reg0, reg1 <=> Reads 64-bit value from address reg1 to reg0
	Load8; load reg0, reg1 <=> Reads 8-bit signed integer from address reg1 to reg0
	Load8u; load reg0, reg1 <=> Reads 8-bit unsigned integer from address reg1 to reg0
	Store; store reg0, reg1 <=> Writes value of reg1 to memory address at reg0
	Store8; store reg0, reg1 <=> Writes first 8 bits of reg1 as signed int to memory address at reg0
	Store8; store reg0, reg1 <=> Writes first 8 bits of reg1 as unsigned int to memory address at reg0
	printi; printi reg0 <=> Prints integer value to stdout. Deprecated and probably replaced by syscalls
	alloc reg0, reg1; <=> Allocates reg1 amount of heap memory bytes and stores pointer to reg0
	Syscall; syscall reg0, reg1 | Performs syscall pointed by reg1 and stores return value to reg0
```


### Syscalls

Syscall is a special instruction that allows the bytecode program to call functions that are outside of regular VM behavior. The VM core supports attaching a syscall table which allows the embedding program to write the syscalls their project might need. This is the mechanism that keeps the VM extendable per project needs.
Syscalls could allow for example interacting with the filesystem, open network sockets and so on, but the VM core has loose coupling on purpose to keep the implementation size small.

Some standard library implementation that can optionally be registered and compiled will be implemented but kept trivial on purpose.

syscalls work as following
```
syscall reg0, 1
```
Where reg0 will receive the syscall return value and 1 defines what syscall number to call. Syscall function receives handle to the NanoVM and is responsible for determining the arguments either from registers or stack. Note that syscall implementations access the VM memory so memory safety is the responsibility of the implementation. Helper functions are provided and usage is recommended. Fuzzing or other tests won't cover custom syscall implementations.

#### Recommended syscall contract

Use reg4-reg6 as first arguments for the syscall to avoid having to push/pop stack memory for functions with low amount of arguments. For more arguments, use NanoVM stack pop functionality, and for pointers to memory use the NanoVM read/write memory helpers as they will ensure memory operations stay within VM memory bounds.


## Heap memory

NanoVM supports heap memory through 'alloc' and 'free' opcodes. As initial version a simple bump list allocator is used => free opcode is no-op.

```asm
alloc reg0, 500 ;allocate 500 bytes in heap
store reg0, 123 ;write to heap
free reg0 ;free heap memory allocation
```

# NanoAssembler
NanoAssembler is currently a rough assembler implementation for NanoVM. The assembler was made to aid in making simple programs and tests. This project is not so much about making a "programming language" but rather the core VM which could be used as the base which some programming language is compiled to. Proper assembler should implement lexer, parser and assembler but the assembler is lower priority for this project.

Currently the assembler supports comments with prefix ';' and uses regex to filter multiple whitespaces to help in processing the input. The assembler also suppors labels which are defined by ':' prefix. This will be mapped to a memory address that points to the next instruction after label. Example:
```assembly
; The assembler supports comments
; The assembler strips multiple whitespaces
;          xor        reg0,     reg1 
; The above line would be translated to the one below. So the assembler is not sensitive with whitespaces
xor reg0, reg0 ; zero out reg0
:label
printi reg0 ; Label points here

inc reg0 ; reg0++
cmp reg0, 0x10 ; compare reg0 to 0x10 in hex which is the same as cmp reg0, 10
; The assembler understands base10 and base16 values
jnz label    ; if reg0 != 10 jump to label
; The above code will print numbers
```


The assembler project's code is quite rough and the development for that will most likely be transferred to a separate repository when the VM core is more stable.

# NanoDebugger

The project contains also a simple command line debugger + disassembler. The debugger inherits the NanoVM core and is capable of stepping through the programs. It also supports:
* Breakpoints
* Goto. This allows you to change the current instruction pointer
* print registers. This will print the current register values and flags set by cmp
* Print stack. This will print the stack memory up to the stack pointer. Each line of the dump will be 8 hex values followed by the same values in ascii separated by |. This allows to easily look at potential ASCII strings in stack as well as 64bit integers.
Todo:
* Add commands for modifying the stack and registers
* Add whole memory dump which will dump all the memory pages including code and stack to the disk.
* Add option to disassemble the whole code and dump to the disk with memory offsets
