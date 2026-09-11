# NanoVM
PoC lightweight x64 VM implementation

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

NanoVM is cross-platform register based turing complete VM with stack memory. The project also includes assembler and debugger with similiar syntax to x86 asm with intel syntax. 
Note that the project is still in very early development and many things including the insturction set and format is a subject to change, so bytecode from previous versions might not work in future. 
The documentation will be updated when changes happen.

The longer term goal of the project is to be embeddable VM with a small bytecode format while maintaining reasonable performance speed. 
Syscall instruction that contains some implemented functions like IO but user can register custom functions as callbacks for different syscall function values will be added eventually when the bytecode format has been finalized. 
This allows one to implement more "outside of the VM" functionality". Performance comparison tests to other languages will be added later. 
Longer term goal is to eventually actually program the Compiler/Assembler in NanoVM bytecode. 

Note that even though the VM does do bounds checking for read write and execute operations on memory these checks are more for catching bugs in the code + avoiding VM crashing, and not so much about hardening the VM. 
Escaping the VM sandbox is likely very trivial. 
However, if you notice a way to read, write or execute memory outside of the VM I'll gladly fix those. 
That being said **!this VM should not be used to run unknown and potentially hostile code!**. 
Also stuff like executing stack memory is currently possible and this is made on purpose to allow dynamic code generation or encryption. 
Read/write/execute permissions to memory pages might be added in future.

## How to build

Build instructions have been tested on Windows and Debian based linux distros

### Windows (Visual Studio 2019)

You need to have Visual Studio 2019 and cmake installed on your system.\
Visual Studio 2019 is compatible with cmake projects so you can build the project by opening the project in visual studio, right click the root CMakeLists.txt -> "Generate Cache for NanoVM". This will generate the cmake cache for you and now you can build the project by selecting 
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

## VM architecture

The VM memory are defined as pages which by default are 4096 bytes each. When initialized the VM bytecode will be placed at the bottom of the allocated memory followed by the stack memory base on the next page. While the VM is similiar to x86 the stack grows up unlike in x86. This can be utilized to dynamically increase the stack memory if required with minimal effort.

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
The VM is register based so the instuctions utilize different registers. Registers are encoded with 3 bits so there are 8 registers in total (the names will change in future):

| Register        | Number        | Description                                  |
| -------------   |:-------------:| --------------------------------------------:|
| Reg0            | 0             | General purpose. Used to store return values |
| Reg1            | 1             | General purpose.                             |
| Reg2            | 2             | General purpose.                             |
| Reg3            | 3             | General purpose.                             |
| Reg4            | 4             | General purpose.                             |
| Reg5            | 5             | General purpose.                             |
| Reg6            | 6             | General purpose.                             |
| SP              | 7             | Stack pointer. Points to the top of the stack|

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
Instructions that use 1 operand encode a register operand in the destination register field with source type Reg, and an immediate operand in the immediate value with source type Immediate. The source register field is unused, so a decoder has to read the source type bit to know which field holds the operand. A register operand is always read at the full 64 bits, because the descriptor byte of this form carries no size. Opcodes that use 1 operand:
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
```
Push and pop always move a full 64 bit slot so that call and ret agree on the size of a return address.

An immediate jump or call target is a relative offset measured from the start of the jump instruction itself, not from the instruction that follows it. A register jump or call target is an absolute address, which is what makes indirect calls work, since a label used with any other instruction also resolves to an absolute address:
```assembly
mov reg0, :func ; absolute address of func
call reg0       ; absolute jump
```
Instructions with 2 operands:
```assembly
	Mov; mov reg0, reg1 <=> reg0 = reg1
	Add; add reg0, reg1 <=> reg0 += reg1
	Sub; sub reg0, reg1 <=> reg0 -= reg1
	And; and reg0, reg1 <=> reg0 &= reg1
	Or;  or reg0, reg1 <=> reg0 |= reg1
	Xor; xor reg0, reg1 <=> reg0 ^= reg1
	Shl; shl reg0, reg1 <=> reg0 <<= reg1 | The shift count is taken modulo 64
	Shr; shr reg0, reg1 <=> logical shift right, shifts in zeroes
	Shr_s; shr_s reg0, reg1 <=> arithmetic shift right, shifts in the sign bit
	Mul; mul reg0, reg1 <=> reg0 *= reg1
	Div; div reg0, reg1 <=> unsigned division
	Div_s; div_s reg0, reg1 <=> signed division
	Mod; mod reg0, reg1 <=> unsigned remainder
	Mod_s; mod_s reg0, reg1 <=> signed remainder
	Cmp; cmp reg0, reg1 | Compares the 2 values as unsigned and sets flags depending on the comparison
	Cmp_s; cmp_s reg0, reg1 | Compares the 2 values as signed
	Load; load reg0, reg1 <=> reg0 = memory at the address in reg1. The source size is the access width, so "load reg0, reg1.8" reads a single byte. The address operand must be a register
	Store; store reg0, reg1 <=> memory at the address in reg0 = reg1. Note that here the destination register holds the address and the source holds the value
	Syscall; syscall reg0, 1 | Performs the syscall named by the source operand and stores its result in the destination register
```

### Signed and unsigned
The encoding carries no sign bit. The opcode alone decides how its operands are interpreted, so any operation whose result differs between a signed and an unsigned reading has a separate `_s` opcode: `div_s`, `mod_s`, `cmp_s` and `shr_s`. Everything else (`add`, `sub`, `mul`, `and`, `or`, `xor`, `shl`, `not`, `mov`) produces the same bits either way and needs no variant.

The same rule decides how a narrow source is widened to 64 bits: an `_s` opcode sign extends its source, every other opcode zero extends it. Jump and call displacements are always sign extended. Because of this the assembler picks an immediate width that the opcode's own extension rule reproduces exactly, so `mov reg0, -1` needs the full 8 bytes while `div_s reg0, -1` fits in one.

Only `cmp` and `cmp_s` write the flags. The three flags are mutually exclusive, so `jg` means strictly greater and `js` strictly smaller.

### Syscalls
The print instructions were replaced by `syscall`, which takes its arguments from the stack and writes its result to the destination register:

| Number | Name | Behaviour |
| ------ |:----:| ---------:|
| 0 | write string | Pops an address and writes the null terminated string at it. Returns the number of bytes written |
| 1 | write integer | Pops a value and writes it as a signed decimal. Returns the number of bytes written |
| 2 | write character | Pops a value and writes its lowest byte as an ASCII character. Returns 1 |
| 3 | exit | Pops a value, stores it in reg0 and halts the VM |

ToDo:
* Add data sections so that string literals can be placed in memory without pushing them a byte at a time
* Add an instruction for converting between integers and floats

# NanoAssembler
NanoAssembler is currently a minimalistic assembler for NanoVM. The assembler was made to aid in making simple programs and tests. This project is not so much about making a "programming language" but rather the core VM which could be used as the base which some programming language is compiled to. When more advanced features will be introduced I'll consider creating a new compiler project and leave the assembler for the low level operations.
Currently the assembler supports comments with prefix ';' and uses regex to filter multiple whitespaces to help in processing the input. The assembler also suppors labels which are defined by ':' prefix. This will be mapped to a memory address that points to the next instruction after label. Example:
```assembly
; The assembler supports comments
; The assembler strips multiple whitespaces
;          xor        reg0,     reg1 
; The above line would be translated to the one below. So the assembler is not sensitive with whitespaces
xor reg0, reg0 ; zero out reg0
:label
printi reg0 ; Label points here
printc '\n' ; The assembler can map characters defined with '' and special characters line \n \r \t to their ascii values
; The above line is the same as printc 10
inc reg0 ; reg0++
cmp reg0, 0x10 ; compare reg0 to 0x10 in hex which is the same as cmp reg0, 10
; The assembler understands base10 and base16 values
jnz label    ; if reg0 != 10 jump to label
; The above code will print numbers
```
ToDo:
* Add macros. These would help to reduce the amount of code that needs to be written.
* Add include tags which would allow to write "standard libraries" which could be included to the project
* Size definitions for registers
* ...

The assembler projects code is not currently clean and the development for that will be most likely be stopped eventually and a new compiler project will be started. Probably with external library for parsing the programming language. I will try and keep the assembler simple

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
